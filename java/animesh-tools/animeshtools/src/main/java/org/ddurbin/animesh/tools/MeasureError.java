package org.ddurbin.animesh.tools;

import com.google.common.collect.Sets;

import java.util.Collection;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
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


    public static void main(String[] args) throws Exception {
        MeasureError measure = new MeasureError();
        String directory = args[0];
        String fileName = args[1];
        State state = StateUtilities.loadState(directory, fileName);

        Map<State.Surfel, Map<State.FrameData, Float>> error = measure.measure(state);

        // Report by frame
        Map<State.FrameData, Float> errorByFrame = new HashMap<>();
        Map<State.Surfel, Float> errorBySurfel = new HashMap<>();

        for (Map.Entry<State.Surfel, Map<State.FrameData, Float>> e : error.entrySet()) {
            State.Surfel surfel = e.getKey();
            float surfelError = 0.0f;


            for (Map.Entry<State.FrameData, Float> f : e.getValue().entrySet()) {
                surfelError += f.getValue();
                float frameError = errorByFrame.computeIfAbsent(f.getKey(), i -> 0.0f);
                errorByFrame.put(f.getKey(), frameError + f.getValue());
            }

            errorBySurfel.put(surfel, surfelError);
        }

        errorByFrame.keySet().stream().sorted(Comparator.comparingLong(fd -> fd.frameIdx)).forEach(fd -> {
            System.out.println(String.format("Frame : %d      Error : %8.1f", fd.frameIdx, errorByFrame.get(fd)));
        });

        errorBySurfel.keySet().stream().sorted(Comparator.comparingLong(s -> s.id)).forEach(s -> {
            System.out.println(String.format("Surfel : %05d      Error : %8.1f", s.id, errorBySurfel.get(s)));
        });
    }

    /**
     * Map a Surfel into all frames in which it apears
     */
    Map<State.FrameData, Pair<Vector3f, Vector3f>>
    projectSurfelToAllFrames(State.Surfel s) {
        Map<State.FrameData, Pair<Vector3f, Vector3f>> projectedSurfelsByFrame = new HashMap<>();

        for (State.FrameData fd : s.frameData) {
            Pair<Vector3f, Vector3f> projectedTangentNormal = projectSurfelToFrame(s, fd);
            projectedSurfelsByFrame.put(fd, projectedTangentNormal);
        }

        return projectedSurfelsByFrame;
    }

    /**
     * Map each surfel into it's correct position and orientation in each frame.
     */
    Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>>
    projectAllSurfelsToAllFrames(State state) {
        Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>> allProjectionsbySurfel = new HashMap<>();

        for (State.Surfel s : state.surfels) {
            Map<State.FrameData, Pair<Vector3f, Vector3f>> projectedSurfelsByFrame = projectSurfelToAllFrames(s);
            allProjectionsbySurfel.put(s, projectedSurfelsByFrame);
        }
        return allProjectionsbySurfel;
    }

    /**
     * Compute a mapping from frame to Surfel
     */
    Map<State.FrameData, Set<State.Surfel>>
    mapSurfelsByFrame(State state) {
        Map<State.FrameData, Set<State.Surfel>> surfelsByFrame = new HashMap<>();
        for (State.Surfel s : state.surfels) {
            for (State.FrameData fd : s.frameData) {
                Set<State.Surfel> set = surfelsByFrame.computeIfAbsent(fd, l -> new HashSet<>());
                set.add(s);
            }
        }
        return surfelsByFrame;
    }

    /*
     * Compute the neighbours for each Surfel in each frame.
     */
    Map<State.Surfel, Map<State.FrameData, Set<State.Surfel>>>
    computeSurfelNeighboursInFrames(State state) {
        Map<State.Surfel, Map<State.FrameData, Set<State.Surfel>>> neighboursInFramesBySurfel = new HashMap<>();

        Map<State.FrameData, Set<State.Surfel>> allSurfelsByFrame = mapSurfelsByFrame(state);

        for (State.Surfel currentSurfel : state.surfels) {
            Set<State.Surfel> currentSurfelNeighbours = currentSurfel.neighbours.stream().map(state.surfels::get).collect(Collectors.toSet());
            Map<State.FrameData, Set<State.Surfel>> currentSurfelNeighboursByFrame = new HashMap<>();

            for (State.FrameData fd : currentSurfel.frameData) {
                // neighbours in this frame is the intersection of neighbours of this surfel and surfels in this frame
                Set<State.Surfel> currentSurfelNeighboursInThisFrame = Sets.intersection(currentSurfelNeighbours, allSurfelsByFrame.get(fd));
                currentSurfelNeighboursByFrame.put(fd, currentSurfelNeighboursInThisFrame);
            }
            neighboursInFramesBySurfel.put(currentSurfel, currentSurfelNeighboursByFrame);
        }
        return neighboursInFramesBySurfel;
    }

    Pair<Vector3f, Vector3f>
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
        return new Pair(targetCandidates[bestTargetIdx], sourceCandidates[bestSourceIdx].times(Math.signum(dp)));
    }

    /*
     * Compute the 4RoSy distance between these pairs of tangent/normal
     */
    float
    computeError(Pair<Vector3f, Vector3f> first, Pair<Vector3f, Vector3f> second) {
        Pair<Vector3f, Vector3f> bestPair = bestRoSyVectorPair(first, second, null);

        float theta = bestPair.first.angleWith(bestPair.second);
        return (theta * theta);
    }

    /*
     * Compute the error between two Surfels in a given frame
     */
    float
    computeSurfelErrorsForFrame(State.Surfel s1,
                                State.Surfel s2,
                                State.FrameData fd,
                                Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>> surfelsInFrames) {
        Pair<Vector3f, Vector3f> first = surfelsInFrames.get(s1).get(fd);
        Pair<Vector3f, Vector3f> second = surfelsInFrames.get(s2).get(fd);
        return computeError(first, second);
    }

    /*
     * Compute error between one surfel and a list of others where the error is the sum of diffs
     */
    float
    computeSurfelErrorsForFrame(State.Surfel s1,
                                Collection<State.Surfel> others,
                                State.FrameData fd,
                                Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>> surfelsInFrames) {
        float error = 0.0f;
        for (State.Surfel s2 : others) {
            error += computeSurfelErrorsForFrame(s1, s2, fd, surfelsInFrames);
        }
        return error;
    }

    /*
     * For each surfel and frame, compute the sum of the deltas between it and its neighbours in the same frame
     */
    Map<State.Surfel, Map<State.FrameData, Float>>
    computeSurfelsErrorsInFrames(State state, Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>> surfelsInFrames) {

        Map<State.Surfel, Map<State.FrameData, Float>> errorPerFrameBySurfel = new HashMap<>();
        Map<State.Surfel, Map<State.FrameData, Set<State.Surfel>>> surfelsNeighboursInFrames = computeSurfelNeighboursInFrames(state);

        for (Map.Entry<State.Surfel, Map<State.FrameData, Set<State.Surfel>>> neighboursInFramesBySurfel : surfelsNeighboursInFrames.entrySet()) {
            State.Surfel currentSurfel = neighboursInFramesBySurfel.getKey();

            Map<State.FrameData, Float> currentSurfelErrorByFrame = new HashMap<>();
            for (Map.Entry<State.FrameData, Set<State.Surfel>> neighboursByFrame : neighboursInFramesBySurfel.getValue().entrySet()) {
                State.FrameData frame = neighboursByFrame.getKey();
                float error = computeSurfelErrorsForFrame(currentSurfel, neighboursByFrame.getValue(), frame, surfelsInFrames);
                currentSurfelErrorByFrame.put(frame, error);
            }
            errorPerFrameBySurfel.put(currentSurfel, currentSurfelErrorByFrame);
        }
        return errorPerFrameBySurfel;
    }

    Map<State.Surfel, Map<State.FrameData, Float>>
    measure(State state) {
        // Get surfel positions in frames
        Map<State.Surfel, Map<State.FrameData, Pair<Vector3f, Vector3f>>> surfelsInFrames = projectAllSurfelsToAllFrames(state);

        return computeSurfelsErrorsInFrames(state, surfelsInFrames);
    }
}
