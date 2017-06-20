
# Global functions available in lua

## The Require function

Intended to be similar to original lua's *require* function

Syntax:
```lua
module = require(ModuleName)
```

If *ModuleName* is not valid exception is raised {TODO: make it lua error, not exception}

currently only internal modules are supported, they cannot be defined inside any loaded container

### Internal modules

* _RuntimeConfiguration_ \
Visible only in `init.lua` scripts. Cannot be accessed in any other situations {TODO: make it not possible to 'save for later'}
Returned object is Engine RuntimeConfiguration.

