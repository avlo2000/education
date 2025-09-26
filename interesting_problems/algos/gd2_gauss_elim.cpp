#include <vector>
#include <cmath>
#include <iostream>

#include <bitset>
#include <ctime>
#include <algorithm>


void gauss_ellim_gf2(std::vector<int> arr, std::vector<int>& triu)
{
    for(size_t i = 0; i < arr.size(); ++i)
    {
        for (size_t j = 0; j < triu.size(); ++j)
        {
            arr[i] = std::min(arr[i], arr[i] ^ triu[j]);
        }
        if(arr[i] != 0)
        {
            triu.push_back(arr[i]);
        }
    }
    // sort(triu.rbegin(), triu.rend());
}

// Reduce a value by the current triangular basis; returns remainder (0 if representable).
int reduce_by_basis(const std::vector<int>& triu, int x) {
    for (int b : triu) {
        int nx = x ^ b;
        if (nx < x) x = nx; // greedy uses pivot if it decreases numeric value (highest bit cleared)
    }
    return x; // 0 means representable
}

std::vector<int> inverse_coords(const std::vector<int>& triu, int x) {
    std::vector<int> coeff(triu.size(), 0);
    int cur = x;
    for (size_t i = 0; i < triu.size(); ++i) {
        int b = triu[i];
        int nx = cur ^ b;
        if (nx < cur) { // we used this basis vector
            coeff[i] = 1;
            cur = nx;
            if (cur == 0) break; // early exit
        }
    }
    if (cur != 0) return {}; // not representable
    return coeff; // order matches triu order (descending pivots)
}

// Reconstruct vector from coefficients (mainly for validation/demo).
int reconstruct_from_coords(const std::vector<int>& triu, const std::vector<int>& coeff) {
    int acc = 0;
    size_t m = std::min(triu.size(), coeff.size());
    for (size_t i = 0; i < m; ++i) if (coeff[i] & 1) acc ^= triu[i];
    return acc;
}

// Attempt Gauss-Jordan inversion over GF(2) for a square n x n matrix whose rows are bitmasks.
// A is modified locally (passed by value) so caller's original matrix can be kept for verification.
// On success, writes inverse rows (bitmasks) into Ainv and returns true; otherwise false.
bool invert_matrix_gf2(std::vector<int> A, std::vector<int>& Ainv, int n) {
    Ainv.assign(n, 0);
    for (int i = 0; i < n; ++i) Ainv[i] = (1 << i); // identity rows

    int row = 0;
    for (int col = 0; col < n && row < n; ++col) {
        int pivot = -1;
        for (int r = row; r < n; ++r) if ((A[r] >> col) & 1) { pivot = r; break; }
        if (pivot == -1) {
            // Column has no pivot => singular (since we want full rank n)
            continue; // keep searching next columns; if rank < n at end => fail
        }
        if (pivot != row) {
            std::swap(A[pivot], A[row]);
            std::swap(Ainv[pivot], Ainv[row]);
        }
        // Eliminate this column from all other rows
        for (int r = 0; r < n; ++r) if (r != row && ((A[r] >> col) & 1)) {
            A[r] ^= A[row];
            Ainv[r] ^= Ainv[row];
        }
        ++row;
    }
    if (row != n) return false; // rank < n
    // Now A should be identity if invertible; Ainv is inverse.
    return true;
}

// Multiply two n x n matrices (rows as bitmasks) over GF(2)
std::vector<int> multiply_gf2(const std::vector<int>& A, const std::vector<int>& B, int n) {
    std::vector<int> C(n, 0);
    for (int i = 0; i < n; ++i) {
        int row = 0;
        int arow = A[i];
        while (arow) {
            int lsb = arow & -arow; // lowest set bit mask
            int col = __builtin_ctz(arow); // index of that bit
            row ^= B[col]; // add row of B corresponding to that column
            arow ^= lsb;
        }
        C[i] = row;
    }
    return C;
}

// Check identity
bool is_identity(const std::vector<int>& M, int n) {
    for (int i = 0; i < n; ++i) {
        int expected = (1 << i);
        if (M[i] != expected) return false;
    }
    return true;
}

void print_matrix(const std::vector<int>& M, int n, const std::string& name) {
    std::cout << name << " (rows as bitmasks):\n";
    for (int i = 0; i < n; ++i) {
        std::cout << std::bitset<16>(M[i]) << '\n';
    }
}

int main()
{
    std::vector<int> arr;
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    int n = 8; // we will also build an 8x8 matrix to attempt inversion
    for (int i = 0; i < n; ++i) {
        arr.push_back(std::rand() % ((1 << n) - 1));
    }
    std::vector<int> triu;
    gauss_ellim_gf2(arr, triu);
    std::cout << triu.size() << std::endl;
    for(int b : triu) {
        std::cout << std::bitset<32>(b) << std::endl;
    }

    // Demo: try to invert each original vector that is representable.
    std::cout << "--- Inversion demo (coordinate recovery) ---\n";
    for (int x : arr) {
        int rem = reduce_by_basis(triu, x);
        if (rem != 0) {
            std::cout << "Not in span: " << std::bitset<32>(x) << '\n';
            continue;
        }
        auto coeff = inverse_coords(triu, x);
        int rec = reconstruct_from_coords(triu, coeff);
        std::cout << "x=" << std::bitset<16>(x)
                  << " coeffs=";
        for (int c : coeff) std::cout << c;
        std::cout << " rec_ok=" << (rec == x) << '\n';
    }

    // Build a random 8x8 matrix with guaranteed invertibility by regenerating until invertible (limit attempts)
    std::vector<int> A; int attempts = 0; const int MAX_ATTEMPTS = 200;
    bool ok = false; std::vector<int> Ainv;
    while (attempts < MAX_ATTEMPTS && !ok) {
        A.assign(n, 0);
        for (int i = 0; i < n; ++i) A[i] = std::rand() & ((1 << n) - 1);
        ok = invert_matrix_gf2(A, Ainv, n);
        ++attempts;
    }
    if (!ok) {
        std::cout << "Failed to generate invertible " << n << "x" << n << " matrix in " << MAX_ATTEMPTS << " attempts.\n";
        return 0;
    }
    print_matrix(A, n, "A");
    print_matrix(Ainv, n, "A^{-1}");

    // Verify: A * A^{-1} and A^{-1} * A
    auto I1 = multiply_gf2(A, Ainv, n);
    auto I2 = multiply_gf2(Ainv, A, n);
    print_matrix(I1, n, "A * A^{-1}");
    print_matrix(I2, n, "A^{-1} * A");
    std::cout << "Left identity ok? " << (is_identity(I1, n) ? "YES" : "NO") << '\n';
    std::cout << "Right identity ok? " << (is_identity(I2, n) ? "YES" : "NO") << '\n';

    return 0;
}