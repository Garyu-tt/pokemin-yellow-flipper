#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../yellow_center_choice.c"

static void test_yes_transition(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse yes = YCenterYes;

    /* Idle -> Offered via offer() */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);

    /* Offered -> Accepted via Yes response */
    assert(yellow_center_choice_process(&state, &yes) == true);
    assert(state == YCenterChoiceAccepted);
    puts("R012C3_YES_TRANSITION_PASS");
}

static void test_no_transition(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse no = YCenterNo;

    /* Idle -> Offered via offer() */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);

    /* Offered -> Declined via No response */
    assert(yellow_center_choice_process(&state, &no) == true);
    assert(state == YCenterChoiceDeclined);
    puts("R012C3_NO_TRANSITION_PASS");
}

static void test_invalid_response(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse invalid = 99;
    YCenterChoiceState snapshot;

    /* Idle -> Offered */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);

    /* Save snapshot to verify state is not mutated on invalid input */
    snapshot = state;

    /* Invalid response (not Yes/No) must be rejected without mutating state */
    assert(yellow_center_choice_process(&state, &invalid) == false);
    assert(state == snapshot); /* state unchanged */

    /* State is still Offered, so valid response still works */
    YCenterResponse no = YCenterNo;
    assert(yellow_center_choice_process(&state, &no) == true);
    assert(state == YCenterChoiceDeclined);
    puts("R012C3_INVALID_RESPONSE_PASS");
}

static void test_null_response(void) {
    YCenterChoiceState state = YCenterChoiceIdle;

    /* Idle -> Offered */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);

    /* Null response pointer must fail without mutating state */
    YCenterChoiceState snapshot = state;
    assert(yellow_center_choice_process(&state, NULL) == false);
    assert(state == snapshot); /* state unchanged */
    puts("R012C3_NULL_RESPONSE_PASS");
}

static void test_null_state(void) {
    YCenterResponse yes = YCenterYes;

    /* Null state pointer must fail */
    assert(yellow_center_choice_offer(NULL) == false);
    assert(yellow_center_choice_process(NULL, &yes) == false);
    yellow_center_choice_reset(NULL); /* should not crash */
    puts("R012C3_NULL_STATE_PASS");
}

static void test_duplicate_offer_rejected(void) {
    YCenterChoiceState state = YCenterChoiceIdle;

    /* First offer succeeds */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);

    /* Duplicate offer in Offered state must be rejected */
    assert(yellow_center_choice_offer(&state) == false);
    assert(state == YCenterChoiceOffered); /* unchanged */
    puts("R012C3_DUPLICATE_OFFER_REJECTED");
}

static void test_duplicate_response_rejected(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse yes = YCenterYes;

    /* Idle -> Offered -> Accepted */
    assert(yellow_center_choice_offer(&state) == true);
    assert(yellow_center_choice_process(&state, &yes) == true);
    assert(state == YCenterChoiceAccepted);

    /* Duplicate response in non-Offered state must fail */
    YCenterChoiceState snapshot = state;
    assert(yellow_center_choice_process(&state, &yes) == false);
    assert(state == snapshot); /* state unchanged */
    puts("R012C3_DUPLICATE_RESPONSE_REJECTED");
}

static void test_accepted_cannot_decline(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse no = YCenterNo;

    /* Idle -> Offered -> Accepted */
    assert(yellow_center_choice_offer(&state) == true);
    /* Offered -> Accepted, then a No response must be rejected. */
    YCenterResponse yes = YCenterYes;
    assert(yellow_center_choice_process(&state, &yes) == true);
    assert(state == YCenterChoiceAccepted);

    /* From Accepted, No response should fail */
    assert(yellow_center_choice_process(&state, &no) == false);
    assert(state == YCenterChoiceAccepted); /* unchanged */
    puts("R012C3_ACCEPTED_NO_FAIL_PASS");
}

static void test_declined_cannot_accept(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse no = YCenterNo;

    /* Idle -> Offered -> Declined */
    assert(yellow_center_choice_offer(&state) == true);
    assert(yellow_center_choice_process(&state, &no) == true);
    assert(state == YCenterChoiceDeclined);

    /* From Declined, Yes response should fail */
    YCenterResponse yes = YCenterYes;
    assert(yellow_center_choice_process(&state, &yes) == false);
    assert(state == YCenterChoiceDeclined); /* unchanged */
    puts("R012C3_DECLINED_YES_FAIL_PASS");
}

static void test_reset_after_accepted(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse yes = YCenterYes;

    assert(yellow_center_choice_offer(&state) == true);
    assert(yellow_center_choice_process(&state, &yes) == true);
    assert(state == YCenterChoiceAccepted);

    yellow_center_choice_reset(&state);
    assert(state == YCenterChoiceIdle);

    /* Fresh offer works after reset */
    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);
    puts("R012C3_RESET_AFTER_ACCEPTED_PASS");
}

static void test_reset_after_declined(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse no = YCenterNo;

    assert(yellow_center_choice_offer(&state) == true);
    assert(yellow_center_choice_process(&state, &no) == true);
    assert(state == YCenterChoiceDeclined);

    yellow_center_choice_reset(&state);
    assert(state == YCenterChoiceIdle);

    assert(yellow_center_choice_offer(&state) == true);
    assert(state == YCenterChoiceOffered);
    puts("R012C3_RESET_AFTER_DECLINED_PASS");
}

static void test_no_offer_idle_process_fails(void) {
    YCenterChoiceState state = YCenterChoiceIdle;
    YCenterResponse yes = YCenterYes;

    /* Process on Idle without offer must fail */
    assert(yellow_center_choice_process(&state, &yes) == false);
    assert(state == YCenterChoiceIdle); /* unchanged */
    puts("R012C3_IDLE_PROCESS_FAILS_PASS");
}

int main(void) {
    test_yes_transition();
    test_no_transition();
    test_invalid_response();
    test_null_response();
    test_null_state();
    test_duplicate_offer_rejected();
    test_duplicate_response_rejected();
    test_accepted_cannot_decline();
    test_declined_cannot_accept();
    test_reset_after_accepted();
    test_reset_after_declined();
    test_no_offer_idle_process_fails();
    puts("R012C3_ALL_TESTS_PASS");
    return 0;
}
