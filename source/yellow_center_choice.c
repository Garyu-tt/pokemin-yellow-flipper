#include "yellow_center_choice.h"

bool yellow_center_choice_offer(YCenterChoiceState* state) {
    if (state == NULL) return false;
    if (*state != YCenterChoiceIdle) return false;
    *state = YCenterChoiceOffered;
    return true;
}

bool yellow_center_choice_process(YCenterChoiceState* state, const YCenterResponse* response) {
    if (state == NULL) return false;
    if (response == NULL) return false;
    if (*state != YCenterChoiceOffered) return false;

    /* Invalid response values are rejected without mutating state. */
    if (*response != YCenterYes && *response != YCenterNo) return false;

    if (*response == YCenterYes) {
        *state = YCenterChoiceAccepted;
    } else {
        *state = YCenterChoiceDeclined;
    }
    return true;
}

void yellow_center_choice_reset(YCenterChoiceState* state) {
    if (state == NULL) return;
    *state = YCenterChoiceIdle;
}
