#include <sys/io.h>
#include <fs.h>
#include <bios.h>
#include <syslinux/memscan.h>
#include <syslinux/firmware.h>

struct firmware *firmware = NULL;

extern struct ansi_ops bios_ansi_ops;

extern void bios_erase(int, int, int, int, uint8_t);
extern void bios_write_char(uint8_t, uint8_t);
extern void bios_showcursor(uint16_t);
extern void bios_scroll_up(uint8_t, uint8_t, uint8_t);
extern void bios_set_cursor(int, int, bool);
extern void bios_beep(void);
extern void bios_set_mode(uint16_t mode);
extern void bios_get_mode(int *rows, int *cols);
extern void bios_get_cursor(int *x, int *y);

struct output_ops bios_output_ops = {
	.erase = bios_erase,
	.write_char = bios_write_char,
	.showcursor = bios_showcursor,
	.set_cursor = bios_set_cursor,
	.scroll_up = bios_scroll_up,
	.beep = bios_beep,
	.get_mode = bios_get_mode,
	.set_mode = bios_set_mode,
	.get_cursor = bios_get_cursor,
};

extern char bios_getchar(void);

struct input_ops bios_input_ops = {
	.getchar = bios_getchar,
};

static const char *syslinux_ipappend_string_list[32];
bool bios_ipappend_strings(char **list, int *count)
{
    static com32sys_t reg;
    int i;

    reg.eax.w[0] = 0x000f;
    __intcall(0x22, &reg, &reg);

    if (reg.eflags.l & EFLAGS_CF)
	return false;

    for (i = 0; i < reg.ecx.w[0]; i++) {
	syslinux_ipappend_string_list[i] =
	    MK_PTR(reg.es,
		   *(uint16_t *) MK_PTR(reg.es, reg.ebx.w[0] + i * 2));
    }

    *list = syslinux_ipappend_string_list;
    *count = reg.ecx.w[0];

    return true;
}

extern char *bios_get_config_file_name(void);
extern void bios_get_serial_console_info(uint16_t *, uint16_t *, uint16_t *);

struct firmware bios_fw = {
	.init = bios_init,
	.scan_memory = bios_scan_memory,
	.adjust_screen = bios_adjust_screen,
	.cleanup = bios_cleanup_hardware,
	.disk_init = bios_disk_init,
	.o_ops = &bios_output_ops,
	.i_ops = &bios_input_ops,
	.ipappend_strings = bios_ipappend_strings,
	.get_config_file_name = bios_get_config_file_name,
	.get_serial_console_info = bios_get_serial_console_info,
};

void syslinux_register_bios(void)
{
	firmware = &bios_fw;
}
