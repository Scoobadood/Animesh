# Fix cycle extraction on cloth.

Cloth samples have far fewer nodes than the 32x32 sphere but seem to take far longer to import.  Given the connectivity is the same, there must be some underlying problem in the code causing this.

## Approach
We'll set up tests in TestGraphCycles to load planar and deformed cloth and explore why it takes so long to parse.


+--+--+--+--+--+
|  |  |  |  |  |
+--+--+--+--+--+
|  |  |  |  |  |
+--+--+--+--+--+
|  |  |  |  |  |
+--+--+--+--+--+
|  |  |  |  |  |
+--+--+--+--+--+
|  |  |  |  |  |
+--+--+--+--+--+

Interrupting the code shows paths of length 14 which are way larger than expected.
Discovered cycles are still only 4 long which is as expected.
Hypothesis:
Due to earlier cycles being extracted, it is not possible to find a path for later nodes in the graph which don't already exist. The algorithm keeps searching because there are unexplored paths however those paths are long and tortuous and exhaustively searching the space is expensive.
How could be disprove this:
We could find the termination conditions for the cycle extraction code.

**Why doesn't this fail in the test cases?**
OK, Stepping through the code we find that it _does_ work for the tier0 graph. However the tier3 one locks up. So there is no point seeking explanations in the tier 0 graph, we need to see how tier3 looks. We should be able to dump this from code.
50 nodes
112 edges

**Simple fix**
Disable singularity computation on tiers which are not 0. These tiers are only for helping with debug anyway and ultimately we won't render them.
