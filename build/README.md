# Fix cycle extraction on cloth.

Cloth samples have far fewer nodes than the 32x32 sphere but seem to take far longer to import.  Given the connectivity is the same, there must be some underlying problem in the code causing this.

## Approach
We'll set up tests in TestGraphCycles to load planar and deformed cloth and explore why it takes so long to parse.


## Refactor UI code
This is a stepping stone on the way. The idea is to be more clear about where the events are triggered, where the model is updated and polled as well as where the UI is instructed to update.

# Singularities
## Change
When a single smooth or complete smooth is performed.
When a frame changes
When the graph changes
When a new file is loaded

## Display
Count of each type
UI overlay

## Compute
When there is a change
Render when draw is on
No explicit update method, just a call to fetch values

## Action
### Factor out singularities_changed() method
If draw is on, get new values and call renders
If draw is off do nothing

### In toggle
When toggled off update views (turn off layer)
When toggled on get new values and call renders

### maybe_update
get new values and call renders
