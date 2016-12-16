/*@ predicate valid_subseq{L}(int *x, integer left, integer right) =
  @     \forall integer i; left <= i < right ==> \at(x[i], L) == \at(x[left], L);
*/

/*@ predicate is_max_subseq{L}(int *x, integer kCandidate, integer left, integer right, integer best) =
  @     \forall integer i; left <= i < right && i != kCandidate ==> !valid_subseq(x, i, i + best);
*/

/*@ predicate some_pred{L}(int *x, integer k, integer best) = 
  @     \at(x[k + best + 1],L) == \at(x[k], L) ? valid_subseq(x, k, k + best) : 0 > 1;
*/

/*@ requires 1 <= N <= 1000000;
  @ requires \valid(x + (0..N-1));
  @ ensures \exists integer k; 0 <= k < N && valid_subseq(x, k, k + \result);
*/
int countSameConsecutive(int N, int x[]) {
    int best = 0, i = 0;
    /*@ loop invariant 0 <= i <= N;
      @ loop invariant i == 0 ==> best == 0;
      @ loop invariant i > 0 ==> 1 <= best <= i;
      @ loop invariant i > 0 ==> \exists integer k; 0 <= k < i && valid_subseq(x, k, k + best);
      @ loop assigns i, best;
      @ loop variant N - i;
    */
    while (i < N) {
        int j = i+1;
        /*@ loop invariant i < j <= N;
          @ loop invariant \forall integer k; i <= k < j ==> x[k] == x[i];
          @ loop assigns j;
          @ loop variant N - j;
        */
        while (j < N && x[j] == x[i]) ++j;
        /*@ assert j < N ==> x[j] != x[i];
         */
        if (j-i > best) best = j-i;
        /*@ assert best >= j - i;
        */
        i = j;
    }
    return best;
}
