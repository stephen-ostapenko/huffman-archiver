#include<bits/stdc++.h>

using namespace std;

int a[15][15];
int n, m;

int ans[10][10] = {2, 1, 1, 1, 3, 3, 3, 2, 2, 1,
                   1, 2, 1, 3, 1, 3, 2, 3, 2, 3,
                   1, 1, 2, 3, 3, 1, 2, 2, 3, 2,
                   1, 3, 3, 3, 2, 2, 2, 1, 1, 1,
                   3, 1, 3, 2, 3, 2, 1, 2, 1, 3,
                   3, 3, 1, 2, 2, 3, 1, 1, 2, 2,
                   3, 2, 2, 2, 1, 1, 1, 3, 3, 1,
                   2, 3, 2, 1, 2, 1, 3, 1, 3, 3,
                   2, 2, 3, 1, 1, 2, 3, 3, 1, 2};

int ans2 [6][9] = {1, 1, 2, 2, 2, 1, 1, 1, 2,
                   1, 2, 1, 2, 1, 2, 1, 2, 1,
                   2, 1, 1, 1, 2, 2, 2, 1, 1,
                   2, 2, 1, 1, 1, 2, 2, 2, 1,
                   2, 1, 2, 1, 2, 1, 2, 1, 2,
                   1, 2, 2, 2, 1, 1, 1, 2, 2};


void f2() {

    int len = 1;
    for (int i = 0; i < 10; i += 2) {

        for (int j = 0; j < 10; j += len * 2) {
            for (int q = j; q < min(10, j + len); ++q)
                a[i][q] = 1;
            for (int q = j + len; q < min(10, j + len + len); ++q)
                a[i][q] = 2;
        }

        for (int j = 0; j < 10; j += len * 2) {
            for (int q = j; q < min(10, j + len); ++q)
                a[i + 1][q] = 2;
            for (int q = j + len; q < min(10, j + len + len); ++q)
                a[i + 1][q] = 1;
        }
        len++;
    }

}

int main() {

    int c;
    cin >> n >> m >> c;
    if (c == 3)
        return 0;

    if (c == 2) {
        f2();
        if (n > m) {
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    cout << ans2[j][j] << " ";
                }
                cout << endl;
            }
            return 0;
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {

                cout << ans2[i][j] << " ";
            }
            cout << endl;
        }
        return 0;
    }

    if (n < m) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                cout << ans[i][j] << " ";
            }
            cout << endl;
        }
    } else {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                cout << ans[j][i] << " ";
            }
            cout << endl;
        }
    }

}