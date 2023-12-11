lv.start()

scr = lv.scr_act()            # default screen object

scr.set_style_bg_color(lv.color(lv.COLOR_GRAY), lv.PART_MAIN | lv.STATE_DEFAULT)

colp = lv.colorwheel(scr, false)
colp.set_size(200, 200)
colp.set_pos(20,20)

logo = lv.img(scr)
logo.set_tasmota_logo()
logo.center()