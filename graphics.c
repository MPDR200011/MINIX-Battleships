#include "graphics.h"
#include <lcom/lcf.h>
#include "vbe_macros.h"

extern uint32_t squareSide;
extern uint32_t tableGap;

static void* low_mem_base_ptr;
static mmap_t low_mem_map;
static vbe_mode_info_t* vbe_mode_info;

static uint32_t vbe_mode;
static uint16_t hres;
static uint16_t vres;
static uint8_t colorDepth;
static uint32_t bytesPerPixel;
static uint8_t redMaskSize;
static uint8_t greenMaskSize;
static uint8_t blueMaskSize;
static unsigned int vram_base; /* VRAM's physical addresss */
static unsigned int
    vram_size; /* VRAM's size, but you can use the frame-buffer size, instead */
static char* video_mem;
static char* aux_buffer;

int getTableCellSize() {
  return squareSide;
}

int getTableCellGap() {
  return tableGap;
}

uint16_t getHres() {
  return hres;
}

uint16_t getVres() {
  return vres;
}

int vbe_alloc_low_mem_vbe_mode_info() {
  low_mem_base_ptr = lm_init(false);
  int tries = 0;
  while ((vbe_mode_info = lm_alloc(sizeof(vbe_mode_info_t), &low_mem_map)) ==
         NULL) {
    tries++;
    if (tries >= 3) {
      return -1;
    }
  }
  return OK;
}

int vbe_alloc_low_mem_vbe_ctrl() {
  low_mem_base_ptr = lm_init(false);
  int tries = 0;
  while (lm_alloc(VBE_INFO_BLOCK_SIZE, &low_mem_map) == NULL) {
    tries++;
    if (tries >= 3) {
      return -1;
    }
  }
  return OK;
}

int vbe_set_mode(uint16_t mode) {
  vbe_mode = mode;
  struct reg86u reg86;
  memset(&reg86, 0, sizeof(reg86));

  reg86.u.b.intno = VBE_INTERRUPT_VECTOR;
  reg86.u.b.ah = VBE_BIOS_AH_REG;
  reg86.u.b.al = BIOS_SET_VBE_MODE;
  reg86.u.w.bx = mode | BIT(14);

  if (sys_int86(&reg86) != OK) {
    printf("sys_int86: failed to send command to bios\n");
    return -1;
  }

  return 0;
}

void*(vg_init)(uint16_t mode) {
  if (vbe_get_vbe_mode_info(mode)) {
    printf("failed to get vbe mode info\n");
    return NULL;
  }

  hres = vbe_mode_info->XResolution;
  vres = vbe_mode_info->YResolution;
  colorDepth = vbe_mode_info->BitsPerPixel;
  bytesPerPixel = (colorDepth + 1) / 8;
  redMaskSize = vbe_mode_info->RedMaskSize;
  greenMaskSize = vbe_mode_info->GreenMaskSize;
  blueMaskSize = vbe_mode_info->BlueMaskSize;
  vram_base = vbe_mode_info->PhysBasePtr;
  vram_size = hres * vres * bytesPerPixel;

  vbe_free_low_memory();

  if (vbe_map_vram() != OK) {
    printf("failed memory map\n");
    return NULL;
  }

  aux_buffer = malloc(hres * vres * bytesPerPixel);

  if (vbe_set_mode(mode) != OK) {
    return NULL;
  }

  return video_mem;
}

int vbe_get_vbe_mode_info(uint16_t mode) {
  if (vbe_alloc_low_mem_vbe_mode_info() != OK) {
    printf("failed to allocate vbe info low memory\n");
    return -1;
  }

  struct reg86u reg86;
  memset(&reg86, 0, sizeof(reg86));
  reg86.u.b.intno = VBE_INTERRUPT_VECTOR;
  reg86.u.b.ah = VBE_BIOS_AH_REG;
  reg86.u.b.al = BIOS_RETURN_VBE_MODE_INFO;
  reg86.u.w.es = PB2BASE(low_mem_map.phys);
  reg86.u.w.di = PB2OFF(low_mem_map.phys);
  reg86.u.w.cx = mode;

  if (sys_int86(&reg86) != OK) {
    printf("sys_int86: failed to send command to bios\n");
    return -1;
  }

  return OK;
}

int vbe_map_vram() {
  int r;
  struct minix_mem_range mr;

  /* Allow memory mapping */
  mr.mr_base = (phys_bytes)vram_base;
  mr.mr_limit = mr.mr_base + vram_size;

  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr))) {
    printf("sys_privctl (ADD_MEM) failed: %d\n", r);
    return -1;
  }

  /* Map memory */

  video_mem = vm_map_phys(SELF, (void*)mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED) {
    printf("couldn't map video memory");
    return -1;
  }

  return 0;
}

void vbe_free_low_memory() {
  lm_free(&low_mem_map);
}

void vg_free_aux_buffer() {
  free(aux_buffer);
}

uint32_t getMSB(int p) {
  return ((p >> 16) & 0x0000FFFF);
}

uint32_t getLSB(int p) {
  return (p & 0x0000FFFF);
}

uint32_t convertFartoLinear(int p) {
  return ((getMSB(p) << 4) + getLSB(p));
}

int vbe_print_controller_info() {
  if (vbe_alloc_low_mem_vbe_ctrl() != OK) {
    printf("failed to allocate ctrl low memory\n");
    return -1;
  }

  VbeInfoBlock* block = (VbeInfoBlock*)low_mem_map.virt;
  block->vbe_signature[0] = 'V';
  block->vbe_signature[1] = 'B';
  block->vbe_signature[2] = 'E';
  block->vbe_signature[3] = '2';

  struct reg86u reg86;
  memset(&reg86, 0, sizeof(reg86));
  reg86.u.b.intno = VBE_INTERRUPT_VECTOR;
  reg86.u.b.ah = VBE_BIOS_AH_REG;
  reg86.u.b.al = BIOS_RETURN_VBE_CTRL_INFO;
  reg86.u.w.es = PB2BASE(low_mem_map.phys);
  reg86.u.w.di = PB2OFF(low_mem_map.phys);

  if (sys_int86(&reg86) != OK) {
    printf("sys_int86: failed to send command to bios\n");
    return -1;
  }

  vg_vbe_contr_info_t ctrl_info;
  memcpy(&ctrl_info.VBESignature, block->vbe_signature, 4);

  ctrl_info.VBEVersion[0] = block->vbe_version;
  ctrl_info.VBEVersion[1] = block->vbe_version >> 8;

  ctrl_info.OEMString = (char*)((uint32_t)low_mem_base_ptr +
                                convertFartoLinear((int)block->oem_string_ptr));

  ctrl_info.VideoModeList =
      (uint16_t*)((uint32_t)low_mem_base_ptr +
                  convertFartoLinear((int)block->mode_list_ptr));

  ctrl_info.TotalMemory = 64 * block->total_memory;
  ctrl_info.OEMVendorNamePtr =
      (char*)((uint32_t)low_mem_base_ptr +
              convertFartoLinear((int)block->oem_vendor_name_ptr));
  ctrl_info.OEMProductNamePtr =
      (char*)((uint32_t)low_mem_base_ptr +
              convertFartoLinear((int)block->oem_product_name_ptr));
  ctrl_info.OEMProductRevPtr =
      (char*)((uint32_t)low_mem_base_ptr +
              convertFartoLinear((int)block->oem_product_rev_ptr));

  vbe_free_low_memory();

  if (vg_display_vbe_contr_info(&ctrl_info) != OK) {
    printf("failed to print ctrl info\n");
    return -1;
  }

  return 0;
}

// Screen drawing
void vg_clear_canvas() {
  memset(aux_buffer, 0, hres * vres * bytesPerPixel);
}

void vg_flip_buffer() {
  memcpy(video_mem, aux_buffer, hres * vres * bytesPerPixel);
}

void fillPixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x < hres && y < vres) {
    uint index = ((y * hres * bytesPerPixel) + x * bytesPerPixel);
    if (bytesPerPixel == 1) {
      aux_buffer[index] = (uint8_t)color;
    } else if (bytesPerPixel == 2) {
      aux_buffer[index + 1] = (uint8_t)(color >> 8);
      aux_buffer[index] = (uint8_t)color;
    } else if (bytesPerPixel == 3) {
      aux_buffer[index + 2] = (uint8_t)(color >> 16);
      aux_buffer[index + 1] = (uint8_t)(color >> 8);
      aux_buffer[index] = (uint8_t)color;
    } else if (bytesPerPixel == 4) {
      aux_buffer[index + 3] = (uint8_t)(color >> 24);
      aux_buffer[index + 2] = (uint8_t)(color >> 16);
      aux_buffer[index + 1] = (uint8_t)(color >> 8);
      aux_buffer[index] = (uint8_t)color;
    }
  }
}

int(vg_draw_hline)(uint16_t i_x, uint16_t i_y, uint16_t len, uint32_t color) {
  for (int x = 0; x < len; x++) {
    fillPixel(i_x + x, i_y, color);
  }
  return 0;
}

int(vg_draw_rectangle)(uint16_t i_x,
                       uint16_t i_y,
                       uint16_t width,
                       uint16_t height,
                       uint32_t color) {
  for (int y = 0; y < height; y++) {
    vg_draw_hline(i_x, i_y + y, width, color);
  }
  return 0;
}

int draw_sprite(const Sprite* sprite, uint16_t x, uint16_t y) {
  for (int row = 0; row < sprite->info.height; row++) {
    for (int col = 0; col < sprite->info.width; col++) {
      uint32_t color = *(uint32_t*)(sprite->info.bytes +
                                    row * sprite->info.width * bytesPerPixel +
                                    col * bytesPerPixel);
      if (color) {
        fillPixel(x + col, y + row, color);
      }
    }
  }
  return 0;
}

int draw_sprite_ship(const Sprite* sprite,
                     uint16_t x,
                     uint16_t y,
                     bool vertical) {
  for (int row = 0; row < sprite->info.height; row++) {
    for (int col = 0; col < sprite->info.width; col++) {
      uint32_t color;
      if (vertical) {
        color = *(uint32_t*)(sprite->info.bytes +
                             (sprite->info.height - row - 1) *
                                 sprite->info.width * bytesPerPixel +
                             col * bytesPerPixel);
        if (color) {
          fillPixel(x + row, y + col, color);
        }
      } else {
        color = *(uint32_t*)(sprite->info.bytes +
                             row * sprite->info.width * bytesPerPixel +
                             col * bytesPerPixel);
        if (color) {
          fillPixel(x + col, y + row, color);
        }
      }
    }
  }
  return 0;
}

void draw_table(uint32_t table[10][10]) {
  int tableXStart = (getHres() / 2) - ((10 * squareSide + 9 * tableGap) / 2);
  int tableYStart = (getVres() / 2) - ((10 * squareSide + 9 * tableGap) / 2);
  int rx = tableXStart;
  int ry = tableYStart;

  for (int y = 0; y < 10; y++) {
    for (int x = 0; x < 10; x++) {
      vg_draw_rectangle(rx, ry, squareSide, squareSide, table[y][x]);
      rx += squareSide + tableGap;
    }
    rx = tableXStart;
    ry += squareSide + tableGap;
  }
}
