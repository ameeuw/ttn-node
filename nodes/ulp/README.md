Tasmota Berry ULP


### Install micropython
```sh
brew install micropython
micropython
import mip
mip.install('github:micropython/micropython-esp32-ulp')
```


### Fill preprocessor database

Get .h files for the specific processor from here: https://github.com/espressif/esp-idf/blob/master/components/soc
Download full folder using: https://download-directory.github.io
Copy to dir, unzip and rename to "soc"

Choose the chip (e.g. "esp32") and fill the DB
```sh
micropython -m esp32_ulp.parse_to_db soc/esp32/include/soc/*.h
```

Help on how to load into defines database: https://github.com/micropython/micropython-esp32-ulp/blob/master/docs/preprocess.rst


### Build binary

`from esp32_ulp import src_to_binary, preprocess`

You need to call the preprocessor using `source = preprocess(source)` before translating the source to binary `binary = src_to_binary(source, cpu="esp32")`
Ready made counter example:

Compile and build an example:
```sh
micropython examples/ulp_hall.py
```

https://github.com/Staars/berry-examples/blob/main/ulp_examples/ulp_gpio_wake.py