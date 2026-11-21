#ifndef YELLOW_CENTER_CHOICE_H
#define YELLOW_CENTER_CHOICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    YCenterChoiceIdle,       /* no offer active */
    YCenterChoiceOffered,    /* nurse is offering full-heal */
    YCenterChoiceAccepted,   /* player accepted healing */
    YCenterChoiceDeclined    /* player declined healing */
} YCenterChoiceState;

typedef enum {
    YCenterYes = 1, /* accept */
    YCenterNo = 2   /* decline */
} YCenterResponse;

/* Advance offer from idle -> offered.
 * Returns true on success (Idle->Offered).
 * Returns false if already in Offered/Accepted/Declined (duplicate offer rejected).
 */
bool yellow_center_choice_offer(YCenterChoiceState* state);

/* Process player's Yes or No response.
 * Transitions Offered -> Accepted (Yes) or Offered -> Declined (No).
 * Returns true on state transition.
 * Returns false if state != Offered, response == NULL, or invalid response value.
 * Invalid inputs fail without mutating state.
 */
bool yellow_center_choice_process(YCenterChoiceState* state, const YCenterResponse* response);

/* Reset to idle (for new nurse interaction). */
void yellow_center_choice_reset(YCenterChoiceState* state);

#endif /* YELLOW_CENTER_CHOICE_H */
