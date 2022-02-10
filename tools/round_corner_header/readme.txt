When macro definition(USE_SOFTWAWRE_CORNER) in kernel4.4/drivers/video/adf/sprd/drv/sprd_round_corner.c or kernel4.14/drivers/gpu/drm/sprd/sprd_round_corner.c is set to 0, you need to accompish tasks according to the following requirements:(panel resolutin:1080*1920 & corner radius: 60 as an example):
1. generate lcd_top_corner.h and lcd_bottom_corner.h which will be used in sprd_round_corner.h
	a. preparations: use photoshop to create 1 32-bit deep bmp picture
	b. go back to your code directory, execute these commands:
		source build/envsetup.h
		lunch                     (choose your project)
		mmm vendor/sprd/modules/dpu/tools/round_corner_header/
	c. after finishing compilation, use command(adb push) to push out/target/product/s9863a1h10/system/bin/round_corner_header into /system/bin of your phone
	d. use command(adb shell) to enter into /system/bin in your phone, then excute command(round_corner_header -f 32.bmp -R 60 -r 1080x1920) to generate header files which will be stroed in /sdcard
	e. use command(adb pull) to pull /sdcard/lcd_top_corner.h and /sdcard/lcd_bottom_corner.h form your phone and move them into your code(kernel4.4/drivers/video/adf/sprd/drv or kernel4.14/drivers/gpu/drm/sprd/s)
2. compile your code to generate dtb.img & boot.img
	a. configure corner-radius in dts files:
		dispc0: dispc@63000000 {
                                compatible = "sprd,display-controller";
                                sprd,corner-radius = <60>;      ------configure corner-radius
                                reg = <0x0 0x63000000 0x0 0x1000>;
                                interrupts = <GIC_SPI 46 IRQ_TYPE_LEVEL_HIGH>;
                                iommus = <&iommu_dispc>;
                                power-domains = <&disp_pd>;
                                status = "disabled";

                                sprd,ip = "dpu-r2p0";
                                sprd,soc = "sharkl3";

                                sprd,syscon-aon-apb = <&aon_apb_controller>;

                                /* output port */
                                port {
                                        dispc0_out: endpoint@0 {
                                                remote-endpoint = <&dsi0_in>;
                                        };
                                };
	b. make sure that USE_SOFTWAWRE_CORNER has been set to 0
	c. make dtbimage & bootimage
