package org.ddurbin.common;

public class Check {
    public static class CheckException extends Exception {
        public CheckException(String message) {
            super(message);
        }
        public CheckException(String template, String ... args) {
            super(String.format(template, args));
        }
    }

    /**
     * Explicitly check that a value is not null
     */
    public static void notNull(Object o, String template, String... args) throws CheckException {
        if( o != null ) {
            return;
        }
        throw new CheckException(template, args);
    }
}
