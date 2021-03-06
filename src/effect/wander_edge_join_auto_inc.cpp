#include "./wander_edge_join_auto_inc.h"
#include "../utility/snake.h"


void WanderEdgeJoinAutoIncEffect::show() {
    for (auto& event: events_) {
        show(event.xDirection, event.yDirection, event.zDirection, 
                event.interval1, event.interval2);
    }
}


void WanderEdgeJoinAutoIncEffect::show(Direction xDirection, Direction yDirection, Direction zDirection,
        int interval1, int interval2)
{
    Call(cube.clear());

    NavigateSnakeAutoIncreace snake1;
    NavigateSnakeAutoIncreace snake2;

    snake1.setDirections({
        yDirection, xDirection, zDirection, util::reverseDirection(xDirection),
        util::reverseDirection(yDirection), util::reverseDirection(zDirection)
    });
    snake2.setDirections({
        xDirection, yDirection, zDirection, util::reverseDirection(yDirection),
        util::reverseDirection(xDirection), util::reverseDirection(zDirection)
    });


    bool invalid = false;;

    if (xDirection == X_ASCEND) {
        if (yDirection == Y_ASCEND) {
            if (zDirection == Z_ASCEND) {
                snake1.add(0, 0, 0);
                snake2.add(0, 0, 0);
            } else if (zDirection == Z_DESCEND) {
                snake1.add(0, 0, 7);
                snake2.add(0, 0, 7);
            } else {
                invalid = true;
            }
        } else if (yDirection == Y_DESCEND) {
            if (zDirection == Z_ASCEND) {
                snake1.add(0, 7, 0);
                snake2.add(0, 7, 0);
            } else if (zDirection == Z_DESCEND) {
                snake1.add(0, 7, 7);
                snake2.add(0, 7, 7);
            } else {
                invalid = true;
            }
        } else {
            invalid = true;
        }
    }
    else if (xDirection == X_DESCEND) {
        if (yDirection == Y_ASCEND) {
            if (zDirection == Z_ASCEND) {
                snake1.add(7, 0, 0);
                snake2.add(7, 0, 0);
            } else if (zDirection == Z_DESCEND) {
                snake1.add(7, 0, 7);
                snake2.add(7, 0, 7);
            } else {
                invalid = true;
            }
        } else if (yDirection == Y_DESCEND) {
            if (zDirection == Z_ASCEND) {
                snake1.add(7, 7, 0);
                snake2.add(7, 7, 0);
            } else if (zDirection == Z_DESCEND) {
                snake1.add(7, 7, 7);
                snake2.add(7, 7, 7);
            } else {
                invalid = true;
            }
        } else {
            invalid = true;
        }
    }
    else {
        invalid = true;
    }

    if (invalid) {
        sleepMs(interval2);
        return;
    }

    while (true) {
        int ret1 = snake1.move();
        int ret2 = snake2.move();
        if (ret1 == Snake::OK && ret1 == ret2) {
            snake1.update();
            //snake2.update();
            sleepMs(interval1);
        }
        else {
            break;
        }
    }

    sleepMs(interval2);
    Call(cube.clear());
}


bool WanderEdgeJoinAutoIncEffect::readFromFP(FILE* fp) {
    while (!feof(fp)) {
        char tag1[32] = { 0 };
        fscanf(fp, "%s", tag1);
        util::toUpperCase(tag1, strlen(tag1));

        if (strcmp(tag1, "<EVENTS>") == 0) {
            while (true) {
                char tag2[32] = { 0 };
                fscanf(fp, "%s", tag2);
                util::toUpperCase(tag2, strlen(tag2));
                if (strcmp(tag2, "<EVENT>") == 0) {
                    char xDirection[12] = { 0 };
                    char yDirection[12] = { 0 };
                    char zDirection[12] = { 0 };
                    int interval1, interval2;

                    fscanf(fp, "%s %s %s %d %d",
                            xDirection, yDirection, zDirection,
                            &interval1, &interval2);

                    events_.emplace_back(
                            util::getDirection(xDirection), util::getDirection(yDirection),
                            util::getDirection(zDirection), interval1, interval2);
                }
                else if (strcmp(tag2, "<END_EVENTS>") == 0) {
                    break;
                }
                else if (strcmp(tag2, "<END>") == 0) {
                    return true;
                }
                else {
                    return false;
                }
            }
        }
        else if (strcmp(tag1, "<END>") == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    return true;
}

