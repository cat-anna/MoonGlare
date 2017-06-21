
# Global functions available in lua

## The Require function

Intended to be similar to original lua's *require* function

Syntax:
```lua
module = require(ModuleName)
```

If *ModuleName* is not valid error is raised

currently only internal modules are supported, they cannot be defined inside any loaded container

### Internal modules

* _RuntimeConfiguration_ \
Visible only in `init.lua` scripts. Cannot be accessed in any other situations, reference to object cannot be store for later use. Returned object is Engine RuntimeConfiguration.


## Logging functions

* print[f]
* hint[f]
* debug[f]
* warning[f]
* error -> also raises lua panic

All of above functions take multiple parameters. Versions with f does string.format instead of behaving like original lua's print function.