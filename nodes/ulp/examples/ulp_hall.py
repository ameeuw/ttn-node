"""
LED-Example with exporting ULP code to Tasmotas Berry implementation
"""
from esp32_ulp import src_to_binary, preprocess
import ubinascii

source = """\
/* ULP Example: Read hall sensor in deep sleep

   For other examples please check:
   https://github.com/espressif/esp-iot-solution/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */


/* ULP assembly files are passed through C preprocessor first, so include directives
   and C macros may be used in these files 
 */

#include "soc/rtc_cntl_reg.h"
#include "soc/rtc_io_reg.h"
#include "soc/soc_ulp.h"
#include "soc/sens_reg.h"

  /* Configure the number of ADC samples to average on each measurement.
     For convenience, make it a power of 2. */
    .set adc_oversampling_factor_log, 2
  .set adc_oversampling_factor, (1 << adc_oversampling_factor_log)

    .set threshold_pos   , 7
    .set threshold_neg   , 2

  /* Define variables, which go into .bss section (zero-initialized data) */
  .bss
  .global Sens_Vp0
Sens_Vp0:
  .long 0 

  .global Sens_Vn0
Sens_Vn0:
  .long 0 

  .global Sens_Vp1
Sens_Vp1:
  .long 0 

  .global Sens_Vn1
Sens_Vn1:
  .long 0 
  .global Sens_Diff_p1
Sens_Diff_p1:
  .long 0

  .global Sens_Diff_n1
Sens_Diff_n1:
  .long 0

  /* Code goes into .text section */
  .text
  .global entry
entry:

  /* SENS_XPD_HALL_FORCE = 1, hall sensor force enable, XPD HALL is controlled by SW */
  WRITE_RTC_REG(SENS_SAR_TOUCH_CTRL1_REG, SENS_XPD_HALL_FORCE_S, 1, 1)

  /* RTC_IO_XPD_HALL = 1, xpd hall, Power on hall sensor and connect to VP and VN */
  WRITE_RTC_REG(RTC_IO_HALL_SENS_REG, RTC_IO_XPD_HALL_S, 1, 1)

  /* SENS_HALL_PHASE_FORCE = 1, phase force, HALL PHASE is controlled by SW */
  WRITE_RTC_REG(SENS_SAR_TOUCH_CTRL1_REG, SENS_HALL_PHASE_FORCE_S, 1, 1)

  /* RTC_IO_HALL_PHASE = 0, phase of hall sensor */
  WRITE_RTC_REG(RTC_IO_HALL_SENS_REG, RTC_IO_HALL_PHASE_S, 1, 0)

  /* SENS_FORCE_XPD_SAR, Force power up */
  WRITE_RTC_REG(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR_S, 2, SENS_FORCE_XPD_SAR_PU)

  /* do measurements using ADC */
  /* r2, r3 will be used as accumulator */
  move r2, 0
  move r3, 0  
  /* initialize the loop counter */
  stage_rst
measure0:
  /* measure Sar_Mux = 1 to get vp0   */
  adc r0, 0, 1
  add r2, r2, r0

  /* measure Sar_Mux = 4 to get vn0 */
  adc r1, 0, 4
  add r3, r3, r1

  /* increment loop counter and check exit condition */
  stage_inc 1
  jumps measure0, adc_oversampling_factor, lt

  /* divide accumulator by adc_oversampling_factor.
     Since it is chosen as a power of two, use right shift */
  rsh r2, r2, adc_oversampling_factor_log

  /* averaged value is now in r2; store it into Sens_Vp0 */
  move r0, Sens_Vp0
  st r2, r0, 0

  /* r3 divide 4 which means rsh 2 bits */
  rsh r3, r3, adc_oversampling_factor_log
  /* averaged value is now in r3; store it into Sens_Vn0 */
  move r1, Sens_Vn0
  st r3, r1, 0
  
  /* RTC_IO_HALL_PHASE = 1, phase of hall sensor */
  WRITE_RTC_REG(RTC_IO_HALL_SENS_REG, RTC_IO_HALL_PHASE_S, 1, 1)

  /* do measurements using ADC */
  /* r2, r3 will be used as accumulator */
  move r2, 0
  move r3, 0  
  /* initialize the loop counter */
  stage_rst
measure1:
  /* measure Sar_Mux = 1 to get vp1   */
  adc r0, 0, 1
  add r2, r2, r0

  /* measure Sar_Mux = 4 to get vn1 */
  adc r1, 0, 4
  add r3, r3, r1

  /* increment loop counter and check exit condition */
  stage_inc 1
  jumps measure1, adc_oversampling_factor, lt

  /* divide accumulator by adc_oversampling_factor.
     Since it is chosen as a power of two, use right shift */
  rsh r2, r2, adc_oversampling_factor_log

  /* averaged value is now in r2; store it into Sens_Vp1 */
  move r0, Sens_Vp1
  st r2, r0, 0

  /* r3 divide 4 which means rsh 2 bits */
  rsh r3, r3, adc_oversampling_factor_log
  /* averaged value is now in r3; store it into Sens_Vn1 */
  move r1, Sens_Vn1
  st r3, r1, 0

/* calculate differences */
    move r3, Sens_Vn1
    ld r3, r3, 0
    move r2, Sens_Vn0
    ld r2, r2, 0
    sub r3, r2, r3         # eventually change to sub r3, r3, r2 for your setup
    move r2, Sens_Diff_n1
    st r3,r2,0
    move r3, Sens_Vp1
    ld r3, r3, 0
    move r2, Sens_Vp0
    ld r2, r2, 0
    sub r3, r3, r2          # eventually change to sub r3, r2, r3 for your setup
    move r2, Sens_Diff_p1
    st r3,r2,0

/* wake up */
    ld r0,r2,0 # Sens_Diff_p1
    JUMPR wake_up, threshold_pos, GE


  /* Get ULP back to sleep */
  .global exit
exit:
  halt

  .global wake_up
wake_up:
  /* Check if the SoC can be woken up */
  READ_RTC_REG(RTC_CNTL_DIAG0_REG, 19, 1)
  and r0, r0, 1
  jump exit, eq

  /* Wake up the SoC and stop ULP program */
  wake
  /* Stop the wakeup timer so it does not restart ULP */
  WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0)
  halt
"""

source = preprocess(source)
binary = src_to_binary(source, cpu="esp32")

# Export section for Berry
code_b64 = ubinascii.b2a_base64(binary).decode('utf-8')[:-1]

# file = open ("ulp_hall.txt", "w")
# file.write(code_b64)

# print("")
# print("#You can paste the following snippet into Tasmotas Berry console:")

output = """
#-
 - Example of ULP driver written in Berry
 -
 - Support for hall sensor of the ESP32
 - Allow wake from deep sleep
 -#

class HALL : Driver
  var p0, p1, n0, n1, diff_p, diff_n
  var thresh_p, thresh_n
  
  def get_code()
    return bytes().fromb64(\""""+code_b64+"""\")
  end

  def init()
    import ULP
    ULP.wake_period(0,1000000)
    ULP.adc_config(0,2,3)
    ULP.adc_config(3,2,3)
    var c = self.get_code()
    ULP.load(c)
    ULP.run()
  end
  
  #- Very specific for the ULP code, greater/equal P difference-#
  def set_thres(threshold)
    import ULP
    var c = self.get_code()
    var jmp_threshold = 51 # can change, when ULP code changes
    var pos = (3+jmp_threshold)*4
    var cmd = c[pos..pos+4]
    cmd.set(0,threshold,2) # upper 16 bit
    ULP.set_mem(51,cmd.get(0, 4))
  end

  #- read from RTC_SLOW_MEM, measuring was done by ULP -#
  def read_voltage()
    import ULP
    self.p0 = ULP.get_mem(59)
    self.n0 = ULP.get_mem(60)
    self.p1 = ULP.get_mem(61)
    self.n1 = ULP.get_mem(62)
    self.diff_p = ULP.get_mem(63)
    self.diff_n = ULP.get_mem(64)
  end

  #- trigger a read every second -#
  def every_second()
    self.read_voltage()
  end

  #- display sensor value in the web UI -#
  def web_sensor()
    import string
    var msg = string.format(
             "{s}<hr>{m}<hr>{e}"
             "{s}Hall sensor{m}ULP readings:{e}"
             "{s}P0 {m}%i{e}"..
             "{s}P1 {m}%i{e}"..
             "{s}Diff P {m}%i{e}"..
             "{s}N0 {m}%i{e}"..
             "{s}N1 {m}%i{e}"..
             "{s}Diff N {m}%i{e}",
                  self.p0, self.p1, self.diff_p, self.n0, self.n1, self.diff_n)
    tasmota.web_send_decimal(msg)
  end

  #- add sensor value to teleperiod -#
  def json_append()
    import string
    var msg = string.format(",\\"Hall\\":{\\"P0\\":%i,\\"P1\\":%i,\\"DP\\":%i,\\"N0\\":%i,\\"N1\\":%i,\\"Dn\\":%i}",
                                 self.p0, self.p1, self.diff_p, self.n0, self.n1, self.diff_n)
    tasmota.response_append(msg)
  end

end

hall = HALL()
tasmota.add_driver(hall)

def usleep(cmd, idx, payload, payload_json)
    import ULP
    ULP.sleep(int(payload))
end
tasmota.add_cmd('usleep', usleep)

def hall_thres(cmd, idx, payload, payload_json)
    import ULP
    import string
    if payload != ""
        hall.set_thres(int(payload))
    end
    tasmota.resp_cmnd(string.format('{"hall threshold":%i}', ULP.get_mem(51)))
end
tasmota.add_cmd('hall_thres', hall_thres)

hall.set_thres(15)
"""

file = open ("dist/ulp_hall.be", "w")
file.write(output)