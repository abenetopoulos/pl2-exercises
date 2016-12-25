/* Frama-c version : Magnesium
 * Wp Version : 0.9
 * alt-ergo version : 1.01
 * to verify, execute:
 * $ frama-c -wp -wp-prover alt-ergo -wp-alt-ergo-opt="-backward-compat" -wp-rte -wp-timeout 20 \
     -wp-verbose 0 count_same.c -then -report
 * */

/*@ predicate valid_subseq{L}(int *x, integer left, integer right) =
  @     \forall integer i; left <= i < right ==> \at(x[i], L) == \at(x[left], L);
*/

/*NOTE(achilles): if we attempt to use the body of the following predicate inline, frama-c cannot validate
                  it...
*/
/*@ predicate best_is_best{L}(int *x, integer left, integer right, integer best) =
        \forall integer l; left <= l && l + best < right && (left < l ==> x[l] != x[l - 1])
                           && valid_subseq(x, l, l + best) ==> x[l] != x[l + best];
  @*/

/*@ requires 1 <= N <= 1000000;
  @ requires \valid(x + (0..N-1));
  @ ensures 1 <= \result <= N;
  @ ensures \exists integer k; 0 <= k < N && valid_subseq(x, k, k + \result);
  @ ensures best_is_best(x, 0, N, \result);
*/
int countSameConsecutive(int N, int x[]) {
    int best = 0, i = 0;

    /*@ loop invariant 0 <= i <= N;
      @ loop invariant i == 0 ==> best == 0;
      @ loop invariant 0 < i ==> 1 <= best <= N;
      @ loop invariant 0 < i < N ==> x[i - 1] != x[i];
      @ loop invariant 0 < i ==> \exists integer k; 0 <= k < i && valid_subseq(x, k, k + best);
      @ loop invariant 0 < i ==> best_is_best(x, 0, i, best);
      @ loop assigns i, best;
      @ loop variant N - i;
    */
    while (i < N) {
        int j = i+1;

        /*@ loop invariant i < j <= N;
          @ loop invariant valid_subseq(x, i, j);
          @ loop assigns j;
          @ loop variant N - j;
        */
        while (j < N && x[j] == x[i]) ++j;

        /*@ assert (j < N ==> (x[j] != x[i] && x[j] != x[j - 1])) || j == N;*/
        if (j-i > best) best = j-i;

        /*@ assert j - i <= best;*/
        /*@ assert !valid_subseq(x, i, i + best) ||
                    (valid_subseq(x, i, i + best) && (i + best <= j && j < N ==> x[i] != x[i + best])) ||
                    (valid_subseq(x, i, i + best) && (j == N ==> 1 == 1));*/
        i = j;
    }

    /*@ assert best <= N;*/
    return best;
}
