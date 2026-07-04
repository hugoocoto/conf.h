# conf.h

Single-file public domain (or MIT licensed) library to abstract the usage of
Lua config files, for C and C++.

## Example 

This can be a casual config file for a GUI project:

```lua
Config = {
    font = {
        size = 26,
    }
}
```

The font size can easily be read using the following C snippet:

```c
Conf conf;
Conf_open(&conf, "config.lua");

double val;
Conf_get_num(conf, &val, "Config.font.size");

Conf_close(conf);
```

## Requirements

- Lua 5.5 (tested; earlier versions may work but are untested)
- Link with `-llua` (or equivalent for your Lua installation)

## Integration

Copy `conf.h` into your project. In exactly one C/C++ file:

```c
#define INCLUDE_CONF_IMPLEMENTATION
#include "conf.h"
```

In other files, just `#include "conf.h"` normally.

## Supported config styles

Both global-table and return-table Lua files work:

```lua
-- Global style
Config = { font = { size = 26 } }
```

```lua
-- Return style
return { font = { size = 26 } }
```

## Error handling

Every Conf_* function return an int, with the status code. CONF
OK (0) is the ok status, other values distinct from zero represent errors. Read
`conf.h` for more details.

That's why we can handle errors in different ways; use the one that better
adapts your style:

1. Ignore the return value (not recommended)
   ```c
   Conf_get_num(conf, &val, "Config.font.size");
   ```

2. Direct assert
   ```c
   assert(Conf_get_num(conf, &val, "Config.font.size") == CONF_OK);
   ```

3. Direct if
   ```c
   if(Conf_get_num(conf, &val, "Config.font.size")){
       // error handling
   }
   ```

4. If not OK
   ```c
   if(Conf_get_num(conf, &val, "Config.font.size") != CONF_OK){
       // error handling
   }
   ```

5. If err = Conf_*()
   ```c
   if((err = Conf_get_num(conf, &val, "Config.font.size"))){
       // error handling
       switch (err){
           case CONF_INVALID:
           ...
       }
   }
   ```
