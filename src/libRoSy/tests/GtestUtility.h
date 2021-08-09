//
// Created by Dave Durbin on 21/5/20.
//

#ifndef ANIMESH_GTESTUTILITY_H
#define ANIMESH_GTESTUTILITY_H

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)

#endif //ANIMESH_GTESTUTILITY_H
