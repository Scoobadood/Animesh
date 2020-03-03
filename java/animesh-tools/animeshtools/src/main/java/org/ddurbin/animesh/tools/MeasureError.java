package org.ddurbin.animesh.tools;

import static org.ddurbin.animesh.tools.State.FrameData;
import static org.ddurbin.animesh.tools.State.Surfel;

import com.google.common.collect.*;

import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import org.ddurbin.common.Pair;
import org.ddurbin.common.Vector3f;

import static org.ddurbin.animesh.tools.StateUtilities.projectSurfelToFrame;

/**
 * The measure of error is the 4RoSy rotation difference between a tangent vector and its neighbours within the same frame.
 * To do this we need, for each frame, to compute the tangent for each surfel.
 * We have a data structure that stores surfels and their transforms for the frames in which they appear as well as their neighbouring surfels
 * But there's lots of redundancy in that data structure and we'd rather not visit every item multiple times
 * <p>
 * So if we iterate over the surfels
 * Then for each one we visit
 * We transform into each distinct frame in which it appears (framedata)
 * -- This is a Map<Map<2D array of surfels x frames containing <tangent/normal> pairs
 * <p>
 * And build a list of neighbours for that frame
 * -- We have a list of neighbouring surfels in the list of neighbours
 * -- We can just copy this list into a target data structure or reference itr later
 * We can then proceed to generate the frame errors:
 * For each surfel
 * For each frame it appears in
 * Store the normal and tangent in that frame
 * Store a list of neighbouring surfels in that frame
 * <p>
 * Map<Surfel, Map<Frame, Pair<tan,norm>>>
 * Map<Surfel, Map<Frame, List<Pair<tan,norm>>>
 */
public class MeasureError {


    private static void mapErrorByFrameAndSurfel(Map<Surfel, Map<Long, Float>> error,
                                                 Map<Long, Float> errorByFrame,
                                                 Map<Surfel, Float> errorBySurfel) {
        int total = error.entrySet().size();
        int count = 0;
        for (Map.Entry<Surfel, Map<Long, Float>> e : error.entrySet()) {
            System.out.print( String.format("\rProcessing %d of %d    ", ++count, total));

            Surfel surfel = e.getKey();
            float surfelError = 0.0f;

            for (Map.Entry<Long, Float> f : e.getValue().entrySet()) {
                surfelError += f.getValue();
                float frameError = errorByFrame.computeIfAbsent(f.getKey(), i -> 0.0f);
                errorByFrame.put(f.getKey(), frameError + f.getValue());
            }

            errorBySurfel.put(surfel, surfelError);
        }
    }

    public static void main(String[] args) throws Exception {
        MeasureError measure = new MeasureError();
        String directory = args[0];
        String fileName = args[1];
        State state = StateUtilities.loadState(directory, fileName);
        System.out.println( "State loaded");

        // Compute the error for each surfel for each frame it appears in.
        Map<Surfel, Map<Long, Float>> error = measure.measure(state);


        Map<Long, Float> errorByFrame = new HashMap<>();
        Map<Surfel, Float> errorBySurfel = new HashMap<>();
        mapErrorByFrameAndSurfel(error, errorByFrame, errorBySurfel);

        // output error by frame.
        errorByFrame.keySet().stream().sorted(Comparator.comparingLong(k->k)).forEach(f -> System.out.println(String.format("Frame : %d      Error : %8.1f", f, errorByFrame.get(f))));

        // output error by surfel.
        errorBySurfel.keySet().forEach(s -> System.out.println(String.format("Surfel : %s      Error : %8.1f", s.id, errorBySurfel.get(s))));
    }

    /**
     * Given a surfel, produce a map from the frames in which it appears to its projected normal and tangent in those frames.
     */
    private Map<Long, Pair<Vector3f, Vector3f>>
    computeTansAndNormsByFrame(Surfel s) {
        Map<Long, Pair<Vector3f, Vector3f>> surfelTanAndNormByFrame = new HashMap<>();

        for (FrameData fd : s.frameData) {
            Pair<Vector3f, Vector3f> projectedTangentNormal = projectSurfelToFrame(s, fd);
            surfelTanAndNormByFrame.put(fd.pixelInFrame.frameIndex, projectedTangentNormal);
        }

        return surfelTanAndNormByFrame;
    }

    /**
     * Map each surfel into it's correct position and orientation in each frame in which it appears.
     */
    private Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>>
    projectAllSurfelsToAllFrames(State state) {
        Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>> allProjectionsbySurfel = new HashMap<>();

        int total = state.surfels.size();
        int count = 0;
        for (Surfel s : state.surfels) {
            System.out.print( String.format("\rProjecting surfel %d of %d    ", ++count, total));
            Map<Long, Pair<Vector3f, Vector3f>> tansAndNormsbyFrame = computeTansAndNormsByFrame(s);
            allProjectionsbySurfel.put(s, tansAndNormsbyFrame);
        }
        return allProjectionsbySurfel;
    }

    /**
     * Construct a mapping from frame number to the surfels that occur in it.
     */
    private SetMultimap<Long, Surfel>
    mapSurfelsByFrame(State state) {

        ImmutableSetMultimap.Builder<Long, Surfel> mapBuilder = ImmutableSetMultimap.builder();

        for (Surfel s : state.surfels) {
            for (FrameData fd : s.frameData) {
                mapBuilder.put(fd.pixelInFrame.frameIndex, s);
            }
        }
        return mapBuilder.build();
    }

    /*
     * Compute the neighbours for each Surfel in each frame.
     */
    private Map<Surfel, SetMultimap<Long, Surfel>>
    computeSurfelNeighboursInFrames(State state) {
        // We'll return this
        ImmutableMap.Builder<Surfel, SetMultimap<Long, Surfel>> neighboursInFramesBySurfel = ImmutableMap.builder();

        // Get a mapping from frame to all the surfels in it.
        SetMultimap<Long, Surfel> allSurfelsByFrame = mapSurfelsByFrame(state);

        // create a map from ID to Surfel
        HashMap<String, Surfel> idToSurfel = Maps.newHashMap();
        for (Surfel currentSurfel : state.surfels) {
            idToSurfel.put(currentSurfel.id, currentSurfel);
        }

        for (Surfel currentSurfel : state.surfels) {
            // Get the current surfel's neighbours into a Set
            Set<Surfel> currentSurfelNeighbours = currentSurfel.neighbours.stream().map(idToSurfel::get).collect(Collectors.toSet());

            ImmutableSetMultimap.Builder<Long, Surfel> currentSurfelNeighboursInFrame = ImmutableSetMultimap.builder();
            for (FrameData fd : currentSurfel.frameData) {
                // Current surfels neighbours in this frame is the intersection of neighbours of this surfel and surfels in this frame
                Set<Surfel> surfelsInThisFrame = allSurfelsByFrame.get(fd.pixelInFrame.frameIndex);
                Set<Surfel> currentSurfelNeighboursInThisFrame = Sets.intersection(currentSurfelNeighbours, surfelsInThisFrame);

                // Add into the by frame MultiMap.
                currentSurfelNeighboursInFrame.putAll(fd.pixelInFrame.frameIndex, currentSurfelNeighboursInThisFrame);
            }
            neighboursInFramesBySurfel.put(currentSurfel, currentSurfelNeighboursInFrame.build());
        }
        return neighboursInFramesBySurfel.build();
    }


    private Pair<Vector3f, Vector3f>
    bestRoSyVectorPair(Pair<Vector3f, Vector3f> source,
                       Pair<Vector3f, Vector3f> target,
                       int[] k) {
        // We'll compare 0 and 90 degree rotations of each vector
        Vector3f[] targetCandidates = {target.first, target.second.cross(target.first)};
        Vector3f[] sourceCandidates = {source.first, source.second.cross(source.first)};

        float bestDotProduct = Float.NEGATIVE_INFINITY;
        int bestTargetIdx = 0;
        int bestSourceIdx = 0;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {

                float dp = Math.abs(targetCandidates[i].dot(sourceCandidates[j]));
                if (dp > bestDotProduct) {
                    bestDotProduct = dp;
                    bestTargetIdx = i;
                    bestSourceIdx = j;
                }
            }
        }

        float dp = targetCandidates[bestTargetIdx].dot(sourceCandidates[bestSourceIdx]);
        if (k != null) {
            k[1] = bestTargetIdx;
            k[0] = (dp >= 0.0f) ? bestSourceIdx : bestSourceIdx + 2;
        }
        return new Pair<>(targetCandidates[bestTargetIdx], sourceCandidates[bestSourceIdx].times(Math.signum(dp)));
    }

    /*
     * Compute the 4RoSy distance between these pairs of tangent/normal
     */
    private float
    computeError(Pair<Vector3f, Vector3f> first, Pair<Vector3f, Vector3f> second) {
        Pair<Vector3f, Vector3f> bestPair = bestRoSyVectorPair(first, second, null);

        float theta = bestPair.first.angleWith(bestPair.second);
        return (theta * theta);
    }

    /*
     * Compute the error between two Surfels in a given frame
     */
    private float
    computeSurfelErrorsForFrame(Surfel s1,
                                Surfel s2,
                                long frameIndex,
                                Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>> surfelsInFrames) {
        Pair<Vector3f, Vector3f> first = surfelsInFrames.get(s1).get(frameIndex);
        Pair<Vector3f, Vector3f> second = surfelsInFrames.get(s2).get(frameIndex);
        return computeError(first, second);
    }

    /*
     * Compute error between one surfel and a list of others where the error is the sum of diffs
     */
    private float
    computeSurfelErrorsForFrame(Surfel s1,
                                Set<Surfel> others,
                                long frameIndex,
                                Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>> surfelsInFrames) {
        float error = 0.0f;
        for (Surfel s2 : others) {
            error += computeSurfelErrorsForFrame(s1, s2, frameIndex, surfelsInFrames);
        }
        return error;
    }

    /*
     * For each surfel and frame, compute the sum of the deltas between it and its neighbours in the same frame
     * @param state The total state.
     * @param surfelsInFrames A map, for each surfel, providing a mapping from frame index to transformed tan and norm in that frame.
     * @return A map from each surfel to the error in each frame.
     */
    private Map<Surfel, Map<Long, Float>>
    computeSurfelsErrorsInFrames(State state, Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>> surfelsInFrames) {


        // Obtain a map from a surfel to the neighbours of that surfel in each frame
        Map<Surfel, SetMultimap<Long, Surfel>> surfelsNeighboursInFrames = computeSurfelNeighboursInFrames(state);

        ImmutableMap.Builder<Surfel, Map<Long, Float>> errorPerFrameBySurfel = ImmutableMap.builder();

        int total = surfelsNeighboursInFrames.entrySet().size();
        int count = 0;


        // For each surfel...
        for (Surfel currentSurfel : surfelsNeighboursInFrames.keySet()) {
            System.out.print( String.format("\rcomputing error for %d of %d    ", ++count, total));

            SetMultimap<Long, Surfel> neighboursByFrame = surfelsNeighboursInFrames.get(currentSurfel);

            // Compute the error for this frme
            Map<Long, Float> currentSurfelErrorByFrame = new HashMap<>();

            // For each frame
            for (Long frame : neighboursByFrame.keys()) {
                Set<Surfel> neighboursInCurrentFrame = neighboursByFrame.get(frame);

                // Compute the error for this frame.
                float error = computeSurfelErrorsForFrame(currentSurfel, neighboursInCurrentFrame, frame, surfelsInFrames);

                // Add this error to the frame.
                currentSurfelErrorByFrame.put(frame, error);
            }
            // Add the errors per frame data to this surfels mapping
            errorPerFrameBySurfel.put(currentSurfel, currentSurfelErrorByFrame);
        }

        return errorPerFrameBySurfel.build();
    }


    /**
     * Compute the error for each surfel for each frame it appears in.
     * Return a map from the surfel to the frame and error.
     */
    private Map<Surfel, Map<Long, Float>>
    measure(State state) {
        // Work out which frames a surfel appears in and project its normal and tangent forward to that frame
        Map<Surfel, Map<Long, Pair<Vector3f, Vector3f>>> surfelsInFrames = projectAllSurfelsToAllFrames(state);

        // For each sufel and frame, compute the error.
        return computeSurfelsErrorsInFrames(state, surfelsInFrames);
    }
}
