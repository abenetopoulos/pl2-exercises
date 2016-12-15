int countSameConsecutive(int N, int x[]) {
    int best = 0, i = 0;
    while (i < N) {
        int j = i+1;
        while (j < N && x[j] == x[i]) ++j;
        if (j-i > best) best = j-i;
        i = j;
    }
    return best;
}
