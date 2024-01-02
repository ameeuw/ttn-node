#- start LVGL and init environment -#
lv.start()

hres = lv.get_hor_res()       # should be 320
vres = lv.get_ver_res()       # should be 240

scr = lv.scr_act()            # default screen object
f20 = lv.montserrat_font(20)  # load embedded Montserrat 20

#- Upper state line -#
stat_line = lv.label(scr)
if f20 != nil stat_line.set_style_text_font(f20, lv.PART_MAIN | lv.STATE_DEFAULT) end
stat_line.set_long_mode(lv.LABEL_LONG_SCROLL)                                        # auto scrolling if text does not fit
stat_line.set_width(hres)
stat_line.set_align(lv.TEXT_ALIGN_LEFT)                                              # align text left
stat_line.set_style_bg_color(lv.color(0xD00000), lv.PART_MAIN | lv.STATE_DEFAULT)    # background #000088
stat_line.set_style_bg_opa(lv.OPA_COVER, lv.PART_MAIN | lv.STATE_DEFAULT)            # 100% background opacity
stat_line.set_style_text_color(lv.color(0xFFFFFF), lv.PART_MAIN | lv.STATE_DEFAULT)  # text color #FFFFFF
stat_line.set_text("L8")
stat_line.refr_size()                                                                # new in LVGL8
stat_line.refr_pos()                                                                 # new in LVGL8

#- display wifi strength indicator icon (for professionals ;) -#
wifi_icon = lv_wifi_arcs_icon(stat_line)    # the widget takes care of positioning and driver stuff
clock_icon = lv_clock_icon(stat_line)

row_y = 60

# --- Styles --- #


btn_style = lv.style()
btn_style.set_radius(10)                        # radius of rounded corners
btn_style.set_bg_opa(lv.OPA_COVER)              # 100% background opacity
if f20 != nil btn_style.set_text_font(f20) end  # set font to Montserrat 20
btn_style.set_bg_color(lv.color(0x1fa3ec))      # background color #1FA3EC (Tasmota Blue)
btn_style.set_border_color(lv.color(0x0000FF))  # border color #0000FF
btn_style.set_text_color(lv.color(0xFFFFFF))    # text color white #FFFFFF

btn_style_2 = lv.style()
btn_style_2.set_radius(10)                        # radius of rounded corners
btn_style_2.set_bg_opa(lv.OPA_COVER)              # 100% background opacity
if f20 != nil btn_style_2.set_text_font(f20) end  # set font to Montserrat 20
btn_style_2.set_bg_color(lv.color(0x1ff3ec))      # background color
btn_style_2.set_border_color(lv.color(0x0000FF))  # border color #0000FF
btn_style_2.set_text_color(lv.color(0xFFFFFF))    # text color white #FFFFFF

# --- Relays --- #

button_relay0 = lv.btn(scr)                                             # create button with main screen as parent
button_relay0.add_flag(lv.OBJ_FLAG_CHECKABLE)                           # create toggle
button_relay0.set_pos(20,row_y)                                         # position of button
button_relay0.set_size(200, 50)                                         # size of button
button_relay0.add_style(btn_style, lv.PART_MAIN | lv.STATE_DEFAULT)     # style of button
button_relay0.add_style(btn_style_2, lv.PART_MAIN | lv.STATE_CHECKED)   # add checked button style
label_relay0 = lv.label(button_relay0)                                  # create a label as sub-object
label_relay0.set_text("RELAY 0")                                        # set label text
label_relay0.center()

button_relay1 = lv.btn(scr)                                             # create button with main screen as parent
button_relay1.add_flag(lv.OBJ_FLAG_CHECKABLE)                           # create toggle
button_relay1.set_pos(20,row_y * 2)                                         # position of button
button_relay1.set_size(200, 50)                                         # size of button
button_relay1.add_style(btn_style, lv.PART_MAIN | lv.STATE_DEFAULT)     # style of button
button_relay1.add_style(btn_style_2, lv.PART_MAIN | lv.STATE_CHECKED)   # add checked button style
label_relay1 = lv.label(button_relay1)                                  # create a label as sub-object
label_relay1.set_text("RELAY 1")                                        # set label text
label_relay1.center()

button_relay2 = lv.btn(scr)                                             # create button with main screen as parent
button_relay2.add_flag(lv.OBJ_FLAG_CHECKABLE)                           # create toggle
button_relay2.set_pos(20,row_y * 3)                                         # position of button
button_relay2.set_size(200, 50)                                         # size of button
button_relay2.add_style(btn_style, lv.PART_MAIN | lv.STATE_DEFAULT)     # style of button
button_relay2.add_style(btn_style_2, lv.PART_MAIN | lv.STATE_CHECKED)   # add checked button style
label_relay2 = lv.label(button_relay2)                                  # create a label as sub-object
label_relay2.set_text("RELAY 2")                                        # set label text
label_relay2.center()

# --- Callbacks --- #

def switch_flipped_cb(obj, event)
    var switch = "Unknown"
    var state = obj.get_state() == 3 ? true : false
    if obj == button_relay0
        switch = "RELAY0"
        tasmota.set_power(0, state)
    elif  obj == button_relay1
        switch = "RELAY1"
        tasmota.set_power(1, state)
    elif  obj == button_relay2
        switch = "RELAY2"
        tasmota.set_power(2, state)

    end
    print(switch, "switch flipped to ", obj.get_state())
end

button_relay0.add_event_cb(switch_flipped_cb, lv.EVENT_VALUE_CHANGED, 0)
button_relay1.add_event_cb(switch_flipped_cb, lv.EVENT_VALUE_CHANGED, 0)
button_relay2.add_event_cb(switch_flipped_cb, lv.EVENT_VALUE_CHANGED, 0)

if (tasmota.get_power(0))
    button_relay0.clear_state(2)
    button_relay0.add_state(3)
else
    button_relay0.clear_state(3)
    button_relay0.add_state(2)
end
if (tasmota.get_power(1))
    button_relay1.clear_state(2)
    button_relay1.add_state(3)
else
    button_relay1.clear_state(3)
    button_relay1.add_state(2)
end
if (tasmota.get_power(2))
    button_relay2.clear_state(2)
    button_relay2.add_state(3)
else
    button_relay2.clear_state(3)
    button_relay2.add_state(2)
end