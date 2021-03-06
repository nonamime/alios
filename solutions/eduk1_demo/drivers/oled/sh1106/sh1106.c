/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#include "sh1106.h"
#include "aos/hal/gpio.h"
#include "aos/hal/spi.h"
#include "hal_iomux_haas1000.h"
#include "sh1106_hdconfig.h"
#include "ulog/ulog.h"
// #include "hal2vfs/io_gpio.h"
// #include "hal2vfs/io_spi.h"
#include <math.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>

#define OLED_PAGE_SIZES 132
#define SPI_MAX_BLOCK   60000

#define PI 3.1415926535

uint8_t OLED_GRAM[8][OLED_PAGE_SIZES];
uint8_t oled_remote_buffer[8 * OLED_PAGE_SIZES * 2];

static spi_dev_t  spi_sh1106 = {0};
static gpio_dev_t gpio_sh1106_dc;
static gpio_dev_t gpio_sh1106_reset;

struct timeval tv;

#if USE_SOFT_SPI == 0
static inline void spi_write(const void *data, uint32_t size)
{
    if (size > SPI_MAX_BLOCK) {
        uint32_t rest  = size;
        uint32_t start = 0;
        while (rest > SPI_MAX_BLOCK) {
            hal_spi_send(&spi_sh1106, (const void *)(data + start),
                         SPI_MAX_BLOCK, 10);
            start += SPI_MAX_BLOCK;
            rest -= SPI_MAX_BLOCK;
        }
        hal_spi_send(&spi_sh1106, (const void *)(data + start), rest, 10);
    } else {
        hal_spi_send(&spi_sh1106, (const void *)data, (uint16_t)size, 10);
    }
}
static inline int OLED_GPIO_Init()
{
    // printf("hardware_init START\r\n");

    gpio_sh1106_dc.port   = SPI1_DC_PIN;
    gpio_sh1106_dc.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&gpio_sh1106_dc);

    gpio_sh1106_reset.port   = SPI1_RESET_PIN;
    gpio_sh1106_reset.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&gpio_sh1106_reset);

    hal_gpio_output_high(&gpio_sh1106_dc);

    hal_gpio_output_low(&gpio_sh1106_reset);
    aos_msleep(1000);
    hal_gpio_output_high(&gpio_sh1106_reset);

    // printf("hal_gpio_init done\r\n");
    return 0;
}

static inline int OLED_SPI_Init()
{
    spi_sh1106.port              = 1;
    spi_sh1106.config.mode       = SPI_WORK_MODE_3; // CPOL = 1; CPHA = 1
    spi_sh1106.config.freq       = 26000000;
    spi_sh1106.config.role       = SPI_ROLE_MASTER;
    spi_sh1106.config.firstbit   = SPI_FIRSTBIT_MSB;
    spi_sh1106.config.t_mode     = SPI_TRANSFER_NORMAL;
    spi_sh1106.config.serial_len = 8;
    spi_sh1106.config.data_size  = SPI_DATA_SIZE_8BIT;
    spi_sh1106.config.cs         = SPI_CS_DIS;

    hal_spi_init(&spi_sh1106);
    printf("hal_spi_init done\r\n");
    return 0;
}

static inline void write_command(uint8_t c)
{
    hal_gpio_output_low(&gpio_sh1106_dc);
    spi_write(&c, sizeof(c));
    hal_gpio_output_high(&gpio_sh1106_dc);
}

static inline void write_data(uint8_t d)
{
    hal_gpio_output_high(&gpio_sh1106_dc);
    spi_write(&d, sizeof(d));
    hal_gpio_output_high(&gpio_sh1106_dc);
}

static inline void write_data_page(uint8_t *data, uint16_t size)
{
    uint8_t i;
    hal_gpio_output_high(&gpio_sh1106_dc);
    spi_write(data, size);
}

#else

static gpio_dev_t oled_gpio;

static inline int OLED_GPIO_Init()
{
    // ?????????PIN???????????????
    oled_gpio.port   = OLED_CLK_PIN;
    oled_gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&oled_gpio);
    oled_gpio.port   = OLED_SDA_PIN;
    oled_gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&oled_gpio);
    oled_gpio.port   = OLED_RES_PIN;
    oled_gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&oled_gpio);
    oled_gpio.port   = OLED_DC_PIN;
    oled_gpio.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&oled_gpio);
    return 0;
}

static inline int OLED_SPI_Init()
{
    // Nothing to do
    OLED_DC(1);

    /* ?????? */
    OLED_RES(0);
    aos_msleep(1000);
    OLED_RES(1);
    return 0;
}

static void OLED_GPIO_Set(unsigned char port, unsigned char leve)
{
    struct gpio_io_config config;
    int                   ret = 0;

    config.id = port;
    config.config = GPIO_IO_OUTPUT | GPIO_IO_OUTPUT_PP;
    if (leve == 1) {
        config.data = 1;
    } else if (leve == 0) {
        config.data = 0;
    }
    ret = ioctl(fd, IOC_GPIO_SET, (unsigned long)&config);
}

void OLED_SCLK(uint8_t x)
{
    if (x == 1) {
        OLED_GPIO_Set(OLED_CLK_PIN, 1);
    } else if (x == 0) {
        OLED_GPIO_Set(OLED_CLK_PIN, 0);
    }
}

void OLED_RES(uint8_t x)
{
    if (x == 1) {
        OLED_GPIO_Set(OLED_RES_PIN, 1);
    } else if (x == 0) {
        OLED_GPIO_Set(OLED_RES_PIN, 0);
    }
}

void OLED_DC(uint8_t x)
{
    if (x == 1) {
        OLED_GPIO_Set(OLED_DC_PIN, 1);
    } else if (x == 0) {
        OLED_GPIO_Set(OLED_DC_PIN, 0);
    }
}

void OLED_SDIN(uint8_t x)
{
    struct gpio_io_config config;
    int                   ret = 0;

    config.id     = 8 * 4 + 0;
    config.config = GPIO_IO_OUTPUT | GPIO_IO_OUTPUT_PP;
    if (x == 0x80) {
        config.data = 1;
    } else if (x == 0) {
        config.data = 0;
    }
    ret = ioctl(fd, IOC_GPIO_SET, (unsigned long)&config);
}

static inline void write_command(uint8_t c)
{
    uint8_t i, k;
    /*???????????????DC */
    OLED_DC(0);

    for (i = 0; i < 8; i++) {
        /* ??????????????????????????? */
        OLED_SCLK(0);
        k = cmd & (0x80);
        OLED_SDIN(k);
        OLED_SCLK(1); // high valid
        cmd <<= 1;
    }
    OLED_DC(1);
}

static inline void write_data(uint8_t d)
{
    uint8_t i, k;
    /* ???????????????DC */
    OLED_DC(1);

    for (i = 0; i < 8; i++) {
        /* ??????????????????????????? */
        OLED_SCLK(0);
        k = data & (0x80);
        OLED_SDIN(k);
        OLED_SCLK(1);
        data <<= 1;
    }
    OLED_DC(1);
}

static inline void write_data_page(uint8_t *data, uint16_t size)
{
    uint8_t i, j, k, d;

    for (j = 0; j < size; j++) {
        d = data[j];
        for (i = 0; i < 8; i++) {
            OLED_SCLK(0);
            k = d & (0x80);
            OLED_SDIN(k);
            OLED_SCLK(1);
            d <<= 1;
        }
    }
    OLED_DC(1);
}
#endif

#if RECORD_SCREEN == 1

static uint16_t zip_oled_gram(uint8_t *des, uint16_t *des_len)
{
    uint8_t buf = OLED_GRAM[0][0];
    uint8_t cnt = 0;
    *des_len    = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 132; j++) {
            if (OLED_GRAM[i][j] == buf) {
                cnt++;
                if (cnt == 0xff) {
                    des[(*des_len)++] = buf;
                    des[(*des_len)++] = cnt;
                    cnt               = 0;
                }
            }
            if (OLED_GRAM[i][j] != buf) {
                des[(*des_len)++] = buf;
                des[(*des_len)++] = cnt;
                cnt               = 1;
                buf               = OLED_GRAM[i][j];
            }
        }
    }
    des[(*des_len)++] = buf;
    des[(*des_len)++] = cnt;
    return *des_len;
}

#endif

void OLED_Refresh_GRAM(void)
{
    for (uint8_t i = 0; i < 8; i++) {
        /* ??????????????????????????? */
        write_command(0xB0 + i); // ????????????????????????
        write_command(0x00);     // ???????????????????????????
        write_command(0x10);     // ???????????????????????????
        write_data_page(OLED_GRAM[i], OLED_PAGE_SIZES);
    }

#if RECORD_SCREEN == 1

    uint16_t zip_len = 0;
    zip_oled_gram(oled_remote_buffer, &zip_len);

    gettimeofday(&tv, NULL);

    if (zip_len <= 8 * 132) {
        // ????????????
        printf("pyszip|%d%d.", tv.tv_sec, tv.tv_usec / 1000);
        printf("%03ld|", tv.tv_usec / 1000);
        uint16_t cnt = 0;
        for (int i = 0; i < zip_len; i++) {
            printf("%02X", oled_remote_buffer[i]);
            cnt += (i % 2 ? oled_remote_buffer[i] : 0);
        }
        printf("|%d\n", cnt);
    } else {
        // ????????????
        printf("pysorg|%d%d.", tv.tv_sec, tv.tv_usec / 1000);
        printf("%03ld|", tv.tv_usec / 1000);
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 132; j++) {
                printf("%02X", OLED_GRAM[i][j]);
            }
        }
        printf("|1056\n");
    }
#endif
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j < OLED_PAGE_SIZES; j++)
            OLED_GRAM[i][j] = 0x0;
}

void OLED_Full(void)
{
    uint8_t i, j;
    for (i = 0; i < 8; i++)
        for (j = 0; j < OLED_PAGE_SIZES; j++)
            OLED_GRAM[i][j] = 0xff;
}

void OLED_Frame_Draw(uint8_t frame[][132])
{
    uint8_t i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < OLED_PAGE_SIZES; j++) {
            OLED_GRAM[i][j] = frame[i][j];
        }
        printf("OLED_GRAM[%d][0] = %02X;\n", i, OLED_GRAM[i][0]);
    }
}

// OLED_Icon_Draw(-10, -10, icon_t *icon, 0)
// 0 ???????????????
// 1 ????????????
// 2 mask??????
void OLED_Icon_Draw(int16_t x, int16_t y, icon_t *icon, uint8_t mode)
{
    int16_t width_byte  = icon->width;
    int16_t height_byte = icon->height / 8 + (icon->height % 8 != 0);
    int16_t column_0    = x;
    int16_t page_0      = (y < 0) ? (y / 8 - (y % 8 != 0)) : (y / 8);
    int8_t  bit_offset  = (y % 8 < 0) ? (y % 8 + 8) : (y % 8);
    if (mode == 2 && icon->p_icon_mask == NULL) {
        mode = 0;
    }
    for (int byte_y = 0; (byte_y < height_byte) && (byte_y + page_0 < 8);
         byte_y++) {
        if (page_0 + byte_y + 1 < 0)
            continue;
        for (int byte_x = 0; (byte_x < width_byte) && (byte_x + column_0 < 132);
             byte_x++) {
            if (byte_x + column_0 < 0)
                continue;
            // ?????????????????? page ?????????
            if (page_0 + byte_y >= 0) {
                if (mode == 1) {
                    OLED_GRAM[page_0 + byte_y][column_0 + byte_x] &=
                        ~(0xff << bit_offset);
                }
                if (mode == 2) {
                    OLED_GRAM[page_0 + byte_y][column_0 + byte_x] &=
                        ~((icon->p_icon_mask[byte_y * width_byte + byte_x])
                          << bit_offset);
                }
                OLED_GRAM[page_0 + byte_y][column_0 + byte_x] |=
                    (icon->p_icon_data[byte_y * width_byte + byte_x])
                    << bit_offset;
            }
            // ????????????????????? page ?????????
            if (bit_offset > 0 && page_0 + byte_y + 1 >= 0 &&
                page_0 + byte_y + 1 < 8) {
                if (mode == 1) {
                    OLED_GRAM[page_0 + byte_y + 1][column_0 + byte_x] &=
                        ~(0xff >> (8 - bit_offset));
                }
                if (mode == 2) {
                    OLED_GRAM[page_0 + byte_y + 1][column_0 + byte_x] &=
                        ~((icon->p_icon_mask[byte_y * width_byte + byte_x]) >>
                          (8 - bit_offset));
                }
                OLED_GRAM[page_0 + byte_y + 1][column_0 + byte_x] |=
                    (icon->p_icon_data[byte_y * width_byte + byte_x]) >>
                    (8 - bit_offset);
            }
        }
    }
}

/* ?????????????????????????????????????????????????????????????????? x:0~127 y:0~63
???x,y???????????????:OLED_GRAM[x][7-y/8]|=1<<(7-y%8)???
mode???1????????????0??????
*/
void OLED_DrawPoint(int16_t x, int16_t y, uint8_t mode)
{
    if ((x > (131)) || (y > 63) || x < 0 || y < 0)
        return;

    uint8_t page       = y / 8;
    uint8_t bit_offset = y % 8;
    uint8_t mask       = 0x01 << bit_offset;

    if (mode == 0)
        OLED_GRAM[page][x] &= ~mask;
    if (mode == 1)
        OLED_GRAM[page][x] |= mask;
    if (mode == 2)
        OLED_GRAM[page][x] ^= mask;
}

/* ??????x,y???????????????/????????????????????????chr
mode???0????????????1???????????????
size???12/16/24
ASCII?????????:
!"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOP
QRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
*/
void OLED_Show_Char(uint8_t x,
                    uint8_t y,
                    uint8_t chr,
                    uint8_t size,
                    uint8_t mode)
{
    /* temp??????????????????????????????????????????y0??????????????? */
    uint8_t temp, t, t1;
    uint8_t y0 = y;
    /* csize???????????????????????????????????? */
    uint8_t csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    /* ???????????????????????????????????????????????????????????????????????????????????? */
    /* chr??????????????????????????????????????? */
    // LOGI("APP", "++++ char=%c size=%d\n", chr, csize);
    chr = chr - ' ';
    /* ???????????????????????????????????? */
    for (t = 0; t < csize; t++) {
        /* ????????????????????????????????????????????????chr??????????????????????????? */
        switch (size) {
            case 12:
                temp = asc2_1206[chr][t];
                break; // 12x6(???x???)
            case 16:
                temp = asc2_1608[chr][t];
                break; // 16x8
            case 24:
                temp = asc2_2412[chr][t];
                break; // 24x12
            default :
                return; // ??????????????????
        }
        // LOGI("APP", " new char x= %d,y=%d,t =%d %d\n", x, y, t, temp);
        /* ????????????????????? */
        for (t1 = 0; t1 < 8; t1++) {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);

            temp <<= 1;
            y++;
            // LOGI("APP", "x= %d,y=%d\n",x,y);
            /* ??????????????????????????????????????????????????????????????????
            ????????????????????????????????????????????? ?????????????????????????????????
            */
            if ((y - y0) == size) {
                // LOGI("APP", "new line\n");
                y = y0;
                x++;
                /* ????????????????????????????????????????????????????????????????????? */
                break;
            }
        }
    }
}
/* ???x,y????????????????????????*p???????????????????????????size????????????????????? */
void OLED_Show_String(uint8_t        x,
                      uint8_t        y,
                      const uint8_t *p,
                      uint8_t        size,
                      uint8_t        mode)
{
    /* ??????????????????????????????????????????????????? */
    while ((*p <= '~') && (*p >= ' ')) {
        /* ????????????????????????????????????????????? */
        if (x > (OLED_PAGE_SIZES - (size / 2))) {
            x = 0;
            y = y + size;
        }
        if (y > (64 - size)) {
            x = y = 0;
            OLED_Clear();
        }

        OLED_Show_Char(x, y, *p, size, mode);
        /* ?????????????????????????????????size/2?????????????????????????????????X????????????
         * ???=2X????????????size???????????? */
        x = x + size / 2;
        p++;
    }
    // OLED_Refresh_GRAM();
}
// end native

static void command_list(void)
{
    /* ??????????????????????????? */
    write_command(0xAE); // ????????????
    write_command(0xD5); // ????????????????????????
    write_command(80);

    write_command(0xA8); // ??????????????????
    write_command(0x3F); // ????????????0x3F???1/64???

    write_command(0xD3); // ??????????????????
    write_command(0x00); // ???????????????0

    write_command(0x40); // ?????????????????????[5:0]

    write_command(0x8D); // ???????????????
    write_command(0x14); // bit2?????????/??????

    write_command(0x20); // ????????????????????????
    write_command(
        0x02); // [1:0],00??????????????????;01??????????????????;10,???????????????;??????10;

    write_command(0xA1); // ??????????????????,bit0:0,0->0;1,0->127;
    write_command(0xC8); // ??????COM????????????;bit3:0,????????????;1,???????????????
                         // COM[N-1]->COM0;N:????????????
    write_command(0xDA); // ??????COM??????????????????
    write_command(0x12); // [5:4]??????

    write_command(0x81); // ???????????????
    write_command(0xEF); // ??????0x7F?????????1~255??????????????????

    write_command(0xD9); // ?????????????????????
    write_command(0xF1); // [3:0],PHASE 1;[7:4],PHASE 2;

    write_command(0xDB); // ??????VCOMH ????????????
    write_command(0x30); // [6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    write_command(0xA4); // ??????????????????;bit0:1,??????;0,??????;(??????/??????)
    write_command(0xA6); // ??????????????????;bit0:1,????????????;0,????????????

    write_command(0xAF); // ????????????

    OLED_Clear();
    OLED_Refresh_GRAM();
    aos_msleep(500);
}

void OLED_DrawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t mode)
{
    uint16_t t;
    int      xerr = 1, yerr = 1, delta_x, delta_y, distance;
    int      incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;

    uRow = x1;
    uCol = y1;
    if (delta_x > 0) {
        incx = 1;
    } else if (delta_x == 0) {
        incx = 0;
    } else {
        incx    = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0) {
        incy = 1;
    } else if (delta_y == 0) {
        incy = 0;
    } else {
        incy    = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x;
    else
        distance = delta_y;

    for (t = 0; t <= distance; t++) {
        OLED_DrawPoint(uRow, uCol, mode);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance / 2) {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance) {
            yerr -= distance;
            uCol += incy;
        }
    }
}

void OLED_DrawLine_ByAngle(int16_t x0,
                           int16_t y0,
                           int16_t angle,
                           int16_t length,
                           uint8_t mode)
{
    int16_t x1 = x0 + length * cos(angle * PI / 180.0);
    int16_t y1 = y0 + length * sin(angle * PI / 180.0);
    OLED_DrawLine(x0, y0, x1, y1, mode);
}

void OLED_DrawHorizontalLine(uint8_t x, uint8_t y, uint8_t length, uint8_t mode)
{
    if (y < 0 || y >= 64) {
        return;
    }

    if (x < 0) {
        length += x;
        x = 0;
    }

    if ((x + length) > 132) {
        length = (132 - x);
    }

    if (length <= 0) {
        return;
    }

    while (length--) {
        OLED_DrawPoint(x + length, y, mode);
    }
}

void OLED_DrawVerticalLine(uint8_t x, uint8_t y, uint8_t length, uint8_t mode)
{
    if (x < 0 || x >= 132) {
        return;
    }

    if (x < 0) {
        length += x;
        x = 0;
    }

    if ((y + length) > 64) {
        length = (64 - x);
    }

    if (length <= 0) {
        return;
    }

    while (length--) {
        OLED_DrawPoint(x, y + length, mode);
    }
}

void OLED_DrawRect(uint8_t x,
                   uint8_t y,
                   uint8_t width,
                   uint8_t height,
                   uint8_t mode)
{
    OLED_DrawHorizontalLine(x, y, width, mode);
    OLED_DrawVerticalLine(x, y, height, mode);
    OLED_DrawVerticalLine(x + width - 1, y, height, mode);
    OLED_DrawHorizontalLine(x, y + height - 1, width, mode);
}

void OLED_FillRect(uint8_t xMove,
                   uint8_t yMove,
                   uint8_t width,
                   uint8_t height,
                   uint8_t mode)
{
    for (uint8_t x = xMove; x < xMove + width; x++) {
        OLED_DrawVerticalLine(x, yMove, height, mode);
    }
}

void OLED_DrawCircle(uint8_t x0,
                     uint8_t y0,
                     uint8_t radius,
                     uint8_t width,
                     uint8_t mode)
{
    OLED_FillCircle(x0, y0, radius, mode);
    OLED_FillCircle(x0, y0, radius - width, !mode);
}

void OLED_FillCircle(uint8_t x0, uint8_t y0, uint8_t r, uint8_t mode)
{
    int x, y;
    int deltax, deltay;
    int d;
    int xi;
    x      = 0;
    y      = r;
    deltax = 3;
    deltay = 2 - r - r;
    d      = 1 - r;

    OLED_DrawPoint(x + x0, y + y0, mode);
    OLED_DrawPoint(x + x0, -y + y0, mode);
    for (xi = -r + x0; xi <= r + x0; xi++)
        OLED_DrawPoint(xi, y0, mode);
    while (x < y) {
        if (d < 0) {
            d += deltax;
            deltax += 2;
            x++;
        } else {
            d += (deltax + deltay);
            deltax += 2;
            deltay += 2;
            x++;
            y--;
        }
        for (xi = -x + x0; xi <= x + x0; xi++) {
            OLED_DrawPoint(xi, -y + y0, mode);
            OLED_DrawPoint(xi, y + y0, mode);
        }
        for (xi = -y + x0; xi <= y + x0; xi++) {
            OLED_DrawPoint(xi, -x + y0, mode);
            OLED_DrawPoint(xi, x + y0, mode);
        }
    }
}

static uint8_t hardware_init(void)
{
    uint8_t ret = 0;

    ret |= OLED_SPI_Init();

    ret |= OLED_GPIO_Init();

    if (ret) {
        printf("hardware_init fail\r\n");
        return ret;
    }

    return ret;
}

uint8_t sh1106_init(void)
{
    uint8_t err_code;

    err_code = hardware_init();
    if (err_code != 0) {
        return err_code;
    }

    command_list();

    return err_code;
}
