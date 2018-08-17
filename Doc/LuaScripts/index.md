
# MoonGlare Lua scripts reference

# Scripts subsystem

TODO: abstract

Scripts should be defined one in lua file, however it is not a requirement.

## Global `Script` object

Global `Script` object manages registration and updates of script classes.

### Global `Script` object Interface

#### `Script:New(NAME)`
Creates and returns new script class identified by *NAME*. If it already exists then returns **nil**.
Provided *NAME* can be used in *Script component* to create its instance.

See [Script class](#scrpt-class) for details on the returned class object and [Script component](#script-component)

#### `Script:Modify(NAME)`
Not tested may not work correctly
TODO
#### `Script:Derive(NAME, BASE)`
This will be implemented in future

## Script class

Script class is lua table returned by call to `Script:New` or `Script:Modify`. It holds are information required to build new script instance. Script class is set as lua **__index metamethod** and all consequences of that can be used.

### Script class interface

User can define any function however MoonGlare Engine will use only specified set of functions.

All functions used by Engine have to be defined using *lua colon* notation because they are called for specified instance. For other functions there is no such requirement

#### Variable default values

Because script class is used as lua **__index** metamethod and **__newindex** metamethod is not used any variable defined or function defined will be used if it is not found in script instance. Thus it will behave as default value for anything defined in script class.
See lua metamethods documentation for details.

in following example function *GetCount* will be returning
counter starting from for each instance created with that script class.

 ```lua
local Class = Script:New("Class")
Class.counter = 5

function Class:GetCounter()
    self.counter = self.counter + 1;
    return self.counter
end
```

#### Class:OnCreate()

This function is called once when *Script component* has been created. It does not take any arguments.

#### Class:OnDestroy()

This function is called once when *Script component* has been destroyed. It does not take any arguments.

**NOTE: Not tested. May not work.**

#### Class:Step(StepData)

This function is called once per engine tick. That means it is called up to 60 times per second.
It takes single argument. See [StepData reference](#stepdata-reference) for details.
Calling of this function may be disabled.

#### Class:PerSecond()

This function is disabled by default. It may be enabled by *Script component* configuration or special function.

This function does not takes any arguments, and if enabled, it is called aproximately once per second.

## GameObject
Basically `GameObject` is container for component instances. It may have no children but always has a parent.

If `GameObject` is removed all its children as removed as well.

### GameObject interface
TODO
#### GameObject:GetComponent(SubSystemId [, Owner])
TODO
#### GameObject:CreateComponent(SubSystemId)
TODO
#### GameObject:SpawnChild(TODO)
TODO
#### GameObject:DestroyObject(TODO)
TODO
#### GameObject:SetName(Name)
TODO
#### GameObject:GetName()
TODO
#### GameObject:FindChild(TODO)
TODO
#### GameObject:GetParent()
TODO
#### GameObject:Destroy()
TODO
#### GameObject:GetObject(internalhandle)
Translates `GameObject` interal handle into lua object which may be used. Returns _nil_ if handle is not valid. See  [Internal handles](#internal-handles) for details.

## Script instance

Script intances are created by *Script component* using registered script classes.

### Script instance api

*Script component* adds some api to script instance during its creation. This api is not a part of script class.

All following functions/variables can be accessed via **self**

Simple example:
```lua
local Class = Script:New("Class")

function Class:OnCreate()
   self:SetStep(false)
end
```

#### self.Entity
This variable holds internal handle to `GameObject` which owns script instance. [Internal handles](#internal-handles) for details.

#### self.Handle
This variable holds script instance handle.

#### self:DestroyComponent()
This function destroys script instance. This action is **irreversible**. Script instance will remain valid until current engine tick ends.

**NOTE: Not tested. May not work.**

#### self:SetPerSecond(value)
This function enables/disables calling of *PerSecond* function. Function must be defined in script class.

#### self:SetStep(value)
This function enables/disables calling of *Step* function. Function must be defined in script class.

#### self:SetActive(value)
This function enables/disables whole script instance.
In disabled state no function will be called from Engine to instance. However internal calls are allowed.

#### Class:GetComponent(SubSystemId [, Owner])
This function is shortcut for [GameObject:GetComponent](#gameobject-getcomponent-componentid-owner-)

#Event subsystem

# Components
TODO
## TODO
TODO
### Script component
TODO
## 3D
TODO
### Transform component
TODO
### Mesh component
TODO
### Camera component
TODO
### Light component
TODO
### Physics component
TODO
### BodyShape component
May emit events (if enabled):
* OnCollisionEnterEvent
* OnCollisionLeaveEvent

## Graphical user interface
TODO
### RectTransform component
TODO
### Panel component
TODO
### Text component
TODO
### Image component
TODO


# Engine modules
TODO
