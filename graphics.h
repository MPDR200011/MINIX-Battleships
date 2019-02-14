#include "lcom/lcf.h"
#include "stdint.h"

/** @defgroup graphics graphics
 * @{
 * 
 */

/**
 * @brief Sprite struct
 * 
 */
typedef struct {
  xpm_image_t info; /**< @brief xpm image representing the sprite. */
} Sprite;

/**
 * @brief Animated sprite struct
 * 
 */
typedef struct {
  Sprite* frames; /**< @brief Frame array. */
  int x; /**< @brief X coordinate of the animation in the screen. */
  int y; /**< @brief Y coordinate of the animation in the screen. */
  int size; /**< @brief Number of frames in the animation. */
  int ticks_to_next_frame; /**< @brief Current tick tracker. */
  int fps; /**< @brief Animation frames per second. */
  int currentframe; /**< @brief Current frame tracker. */
  bool looping; /**< @brief If animation is looping. */
} AniSprite;

#pragma pack(1)
/**
 * @brief VBE information block
 * 
 */
typedef struct {
  char vbe_signature[4];
  short vbe_version;
  uint32_t* oem_string_ptr;
  char capabilities[4];
  uint16_t* mode_list_ptr;
  short total_memory;
  short oem_software_rev;
  char* oem_vendor_name_ptr;
  char* oem_product_name_ptr;
  char* oem_product_rev_ptr;
  char reserved[222];
  char oem_data[256];
} VbeInfoBlock;

#pragma options align = reset

/**
 * @brief Get the screen Width.
 * 
 * @return uint16_t Screen width.
 */
uint16_t getHres();
/**
 * @brief Get the screen heigth.
 * 
 * @return uint16_t Screen height.
 */
uint16_t getVres();
/**
 * @brief Allocate low memory for VBE info extraction.
 * 
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_alloc_low_mem();
/**
 * @brief Set minix to text mode.
 * 
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_set_text_mode();
/**
 * @brief Change minic to a specified video mode.
 * 
 * @param mode Video mode to change to.
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_set_mode(uint16_t mode);
/**
 * @brief Retrieves the vbe information about a certain mode from the BIOS.
 * 
 * @param mode Mode.
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_get_vbe_mode_info(uint16_t mode);
/**
 * @brief Map the video ram to a virtual pointer.
 * 
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_map_vram();
/**
 * @brief Call lm_free().
 * 
 */
void vbe_free_low_memory();
/**
 * @brief Frees the memory occupied by the secondary video buffer.
 * 
 */
void vg_free_aux_buffer();
/**
 * @brief Prints de vbe controller information.
 * 
 * @return int 0 on success, non-zero otherwise.
 */
int vbe_print_controller_info();

/**
 * @brief Clear the secondary buffer.
 * 
 */
void vg_clear_canvas();
/**
 * @brief Flip the buffers.
 * 
 * Copies the secondary buffer to the main video memory, showing the rendered frame.
 * 
 */
void vg_flip_buffer();
/**
 * @brief Draw a sprite.
 * 
 * @param sprite Sprite to draw.
 * @param x X coordinate of where to draw on the window.
 * @param y Y coordinate of where to draw on the window.
 * @return int 0 on success.
 */
int draw_sprite(const Sprite* sprite, uint16_t x, uint16_t y);
/**
 * @brief Draw a ship sprite
 * 
 * Basically draw_sprite with the ability to draw a 90 degree clockwise rotation of the sprite.
 * 
 * @param sprite Ship sprite. 
 * @param x X coordinate of where to draw on the window.
 * @param y X coordinate of where to draw on the window.
 * @param vertical true if you want to rotate the sprite.
 * @return int 0 on success.
 */
int draw_sprite_ship(const Sprite* sprite,
                     uint16_t x,
                     uint16_t y,
                     bool vertical);
/**
 * @brief Draw a board.
 * 
 * Draws a board that is supplied as a bidimensional array of 10x10 ints representing the color of each cell.
 * The board is centered on screen.
 * 
 * @param table The board.
 */
void draw_table(uint32_t table[10][10]);

/**@}*/
