# Fix include in smooth CBs
When there are multiple frames, we should be able to exclude any of them from the smooth.  Checkboxes should be enabled once data is loaded.

Checkboxes should all be checked to start with.

Checkboxes should remain enabled until there is only one left checked. This one cannot be unchecked and should be disabled.

## Step 1.
Make sure they default to disabled in creator file

## Step 2.
On loading, enable them all and check them all.

## Step 3.
On unchecking a checkbox, count how many are still checked. If it's 1, disable that checked box.

## Step 4.
On checking a box, count how many are checked If it's more than 1, enable all check boxes.
