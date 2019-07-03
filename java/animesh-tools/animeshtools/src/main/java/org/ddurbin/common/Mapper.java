package org.ddurbin.common;

public class Mapper {
    public static Vector3f mapVector3f(String string) {
        String[] parts = string.split(",");
        if (parts.length != 3) {
            throw new IllegalArgumentException(String.format("[%s] does not have x, y and z", string));
        }
        try {
            float x = Float.valueOf(parts[0]);
            float y = Float.valueOf(parts[1]);
            float z = Float.valueOf(parts[2]);
            return new Vector3f(x, y, z);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(String.format("[%s] has non-numeric x, y or z", string));
        }

    }

    public static Vector2f mapVector2f(String string) {
        String[] parts = string.split(",");
        if (parts.length != 2) {
            throw new IllegalArgumentException(String.format("[%s] does not have x and y", string));
        }
        try {
            float x = Float.valueOf(parts[0]);
            float y = Float.valueOf(parts[1]);
            return new Vector2f(x, y);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(String.format("[%s] has non-numeric x or y", string));
        }
    }
}
