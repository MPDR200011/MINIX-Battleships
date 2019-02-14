#ifndef BIT
#define BIT(n) (0x01 << (n))
#endif

/** @defgroup i8042 i8042
 * @{
 * 
 * Constants to program the keyboards controller
 * 
 */

#define KBC_DELAY 50 /**< @brief Poling delay */

#define TWO_BYTE_SCAN 0xE0 /**< @brief Byte that idicates that scan code is a two byte scan code. */

#define MAKE_CODE_BIT (BIT(7)) /**< @brief Make code bit mask. */

#define MAKE_CODE_SPEC 0x80 /**< @brief Make code mask. */

#define ESC_BREAK 0x81  
#define R_MAKE_CODE 0x13
#define P_MAKE_CODE 0x19
#define G_MAKE_CODE 0x22
#define S_MAKE_CODE 0x1F
#define L_MAKE_CODE 0x26
#define V_MAKE_CODE 0x2F
#define D_MAKE_CODE 0x20
#define ONE_MAKE_CODE 0x02
#define TWO_MAKE_CODE 0x03
#define THREE_MAKE_CODE 0x04
#define FOUR_MAKE_CODE 0x05
#define FIVE_MAKE_CODE 0x06
#define UP_ARROW_MAKE_CODE 0x48
#define DOWN_ARROW_MAKE_CODE 0x50
#define ENTER_MAKE_CODE 0x1C

#define KBD_IRQ 1  /**< @brief Keyboard irq line. */
#define MOUSE_IRQ 12 /**< @brief Mouse irq line. */

#define STATUS_REG 0x64 /**< @brief Controller status register address. */
#define KBC_CMD_PORT 0x64  /**< @brief Command input register address. */
#define KBC_CMDARG_PORT 0x60 /**< @brief Command argument input register address. */
#define KBC_OUT_BUFFER 0x60 /**< @brief Output buffer register address. */
#define KBC_RESET_MINIX_INTER 0x47 /**< @brief Default control byte. */

#define KBC_READ_CMD 0x20 /**< @brief KBC read command byte command. */
#define KBC_WRITE_CMD 0x60 /**< @brief KBC write command byte command. */

#define OBF BIT(0) /**< @brief Status register Output buffer full bit. */
#define IBF BIT(1) /**< @brief Status register Input buffer full bit. */
#define AUX BIT(5) /**< @brief Status register Mouse data bit. */
#define TIMEOUT BIT(6) /**< @brief Status register Timeout error bit. */
#define PRT BIT(7) /**< @brief Status register Parity error bit. */ 

#define KBC_CMDB_INT BIT(0) /**< @brief KBC Command Byte enable interrupts from keyboard bit. */
#define KBC_CMDB_INT2 BIT(1) /**< @brief KBC Command Byte enable interrupts from mouse bit. */
#define KBC_CMDB_DIS BIT(4) /**< @brief KBC Command Byte disable keyboard bit. */
#define KBC_CMDB_DIS2 BIT(5) /**< @brief KBC Command Byte disable mouse bit. */

#define KBC_CHECK 0xAA /**< @brief KBC self check command. */
#define KBC_OK 0x55 /**< @brief KBC self check OK. */ 
#define KBC_ERROR 0xFC /**< @brief KBC self check ERROR. */

#define KBC_KBD_CHECK_ITF 0xAB /**< @brief KBC check keyboard interface. */ 
#define KBD_ENABLE 0xAE /**< @brief Enable keyboard interface. */       
#define KBD_DISABLE 0xAD /**< @brief Disable keyboard interface. */     

#define KBC_CMDB_ENABLE_KEYBOARD 0x47
#define KBC_ENABLE_MOUSE 0xA8 /**< @brief KBC enable mouse command. */
#define KBC_DISABLE_MOUSE 0xA7 /**< @brief KBC disabke mouse commadn. */
#define KBC_CHECK_MOUSE_ITF 0xA9 /**< @brief KBC check mouse interface command. */
#define KBC_MOUSE_WRITE_BYTE 0xD4 /**< @brief KBC write mouse byte command. */

#define MOUSE_YOVF BIT(7) /**< @brief Mouse Y overflow bit. */
#define MOUSE_XOVF BIT(6) /**< @brief Mouse X overflow bit. */
#define MOUSE_YSIGN BIT(5)/**< @brief Mouse Y sign bit. */
#define MOUSE_XSIGN BIT(4)/**< @brief Mouse X sign bit. */
#define MOUSE_MB BIT(2)/**< @brief Mouse middle button down bit. */
#define MOUSE_RB BIT(1)/**< @brief Mouse right button down bit. */
#define MOUSE_LB BIT(0)/**< @brief Mouse left button down bit. */

#define MOUSE_RESET 0xFF/**< @brief Mouse reset command. */
#define MOUSE_RESEND 0xFE/**< @brief Mouse resend byte commadn. */
#define MOUSE_SET_DEFAULTS 0xF6/**< @brief Mouse set defaults command. */
#define MOUSE_DISABLE_DATA_REP 0xF5/**< @brief Mouse disalble data reporting command. */
#define MOUSE_ENABLE_DATA_REP 0xF4/**< @brief Mouse enable data reporting command. */
#define MOUSE_SET_SAMPLE_RATE 0xF3/**< @brief Mouse set samle rate command. */
#define MOUSE_SET_REMOTE_MODE 0xF0/**< @brief Mouse set remote mode command. */
#define MOUSE_READ_DATA 0xEB/**< @brief Mouse read data command. */
#define MOUSE_SET_STREAM_MODE 0xEA/**< @brief Mouse set stream mode command. */
#define MOUSE_STATUS_REQUEST 0xE9/**< @brief Mouse status request command. */
#define MOUSE_SET_RESOLUTION 0xE8/**< @brief Mouse set resolution command. */
#define MOUSE_SET_ACCELERATION_MODE 0xE7/**< @brief Mouse set acceleration mode command. */
#define MOUSE_SET_LINEAR_MODE 0xE6/**< @brief Mouse set linear mode command. */

#define MOUSE_BYTE_ACK 0xFA/**< @brief Mouse acknoledged byte. */
#define MOUSE_BYTE_NACK 0xFE/**< @brief Mouse not acknoledged byte. */
#define MOUSE_BYTE_ERROR 0xFC/**< @brief Mouse error byte. */

/**@}*/
