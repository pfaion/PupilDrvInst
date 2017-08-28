#include "getopt.h"
#include <stdio.h>
#include "libwdi.h"

int main(int argc,  char *argv[])
{
	int option_index = 0;
	int install_driver = 0;
	char *vendor = "Pupil Labs"; //Default, in case of no cmd line arguments
	char *desc = "Pupil Cam ID0";
	int vid = 0x05A3;
	int pid = 0x9230;
	wdi_options_create_list cl_options = { 0 };
	cl_options.list_all = 1;
	cl_options.list_hubs = 1;
	int c;
	int iface = 0;
	
	char *inf_name;
	while (1) {
		static struct option long_options[] = {
			// These options set a flag.
			{ "inst", no_argument, &install_driver, 'i' },
			{ "vendor", required_argument, 0, 'v' },
			{ "desc", required_argument, 0, 'd' },
			{ "iface", required_argument, 0, 'a' },
			{ "vid", required_argument, 0, 'b' },
			{ "pid", required_argument, 0, 'c' },
			{ 0, 0, 0, 0 }
		};

		c = getopt_long(argc, argv, "a:b:c:v:d:i", long_options, &option_index);
		//  Detect the end of the options.
		if (c == -1)
			break;

		switch (c)
		{
		case 0:
			// If this option set a flag, do nothing else now.
			if (long_options[option_index].flag != 0) {
				break;
			}
			printf("option %s", long_options[option_index].name);
			if (optarg) {
				printf(" with arg %s", optarg);
			}
			printf("\n");
			break;
		case 'a':
			iface = (unsigned char)atoi(optarg);
			printf("OPT: interface number %d\n", iface);
			break;
		case 'b':
			vid = (unsigned short)strtol(optarg, NULL, 0);
			printf("OPT: VID number %d\n", vid);
			break;
		case 'c':
			pid = (unsigned short)strtol(optarg, NULL, 0);
			printf("OPT: PID number %d\n", pid);
			break;
		case 'i':
			inf_name = (char*)optarg;
			break;
		case 'v':
			vendor = (char*)optarg;
			break;
		case 'd':
			desc = (char*)optarg;
			break;
		default:
			break;
			//abort();
		}
	}

	int ret;
	wdi_device_info *dev_list;
	ret = wdi_create_list(&dev_list, &cl_options);

	if (WDI_SUCCESS != ret) {
		return 1;
	}
	int ret_code = 0;
	wdi_device_info *dev;
	wdi_options_prepare_driver prep_driver = { 0 };
	prep_driver.driver_type = WDI_LIBUSBK;
	prep_driver.vendor_name = vendor;
	char *path = "win_drv";
	char *inf = "cam_dev.inf";
	for (dev = dev_list; dev != NULL; dev = dev->next) {
		if (dev->pid == pid && dev->vid == vid && !dev->is_composite) {
			char *prev_desc = dev->desc;
			if (strcmp(dev->driver, "libusbK")) {
				ret_code |= 2;
			}
			
			if (install_driver) {
				dev->desc = desc;
				ret = wdi_prepare_driver(dev, path, inf, &prep_driver);
				if (WDI_SUCCESS != ret) {
					ret_code |= 1;
					goto end;
				}
				ret = wdi_install_driver(dev, path, inf, NULL);
				if (WDI_SUCCESS != ret) {
					ret_code |= 1;
					goto end;
				}
			}
end:
			
			dev->desc = prev_desc;
		}
	}


	wdi_destroy_list(dev_list);
	return ret_code;
}
	
