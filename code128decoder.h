#pragma once

#include "code128patterns.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace code128 {

    using namespace std;

    namespace detail {

        struct Match {
            int value = -1;
            double err = 1e9;
        };

        inline Match matchSymbol(const int* r, int count, int lo, int hi)
        {
            double total = 0;
            for (int i = 0; i < count; ++i)
                total += r[i];
            if (total <= 0)
                return {};

            double divisor;
            if (count == 6)
                divisor = 11.0;
            else
                divisor = 13.0;
            const double unit = total / divisor;

            Match best;
            for (int v = lo; v <= hi; ++v) {
                const char* p = kPatterns[v];
                double err = 0, maxDev = 0;
                for (int i = 0; i < count; ++i) {
                    double d = fabs(r[i] / unit - (p[i] - '0'));
                    err += d;
                    maxDev = max(maxDev, d);
                }
                if (maxDev < 0.85 && err < best.err) {
                    best.err = err;
                    best.value = v;
                }
            }
            return best;
        }

        inline bool valuesToText(const vector<int>& v, string& out)
        {
            if (v.size() < 3)
                return false;

            int set;
            if (v[0] == kStartA)
                set = 0;
            else if (v[0] == kStartB)
                set = 1;
            else
                set = 2;

            bool shift = false;
            out.clear();
            for (size_t i = 1; i + 1 < v.size(); ++i) {
                const int c = v[i];
                int cur = set;
                if (shift && set != 2) {
                    if (set == 0)
                        cur = 1;
                    else
                        cur = 0;
                    shift = false;
                }
                if (cur == 2) {
                    if (c < 100) {
                        out += static_cast<char>('0' + c / 10);
                        out += static_cast<char>('0' + c % 10);
                    }
                    else if (c == 100) {
                        set = 1;
                    }
                    else if (c == 101) {
                        set = 0;
                    }
                   
                }
                else if (c == 99) {
                    set = 2;
                }
                else if (c == 98) {
                    shift = true;
                }
                else if (c == 100) {
                    if (cur == 0) 
                        set = 1; 
                }
                else if (c == 101) {
                    if (cur == 1) 
                        set = 0; 
                }
                else if (c >= 96) {
                    continue;
                }
                else if (cur == 0) {
                    if (c < 64)
                        out += static_cast<char>(32 + c);
                    else
                        out += static_cast<char>(c - 64);
                }
                else {
                    out += static_cast<char>(32 + c);
                }
            }
            return !out.empty();
        }

        inline bool decodeFromRuns(const vector<int>& runs, size_t start, string& out)
        {
            if (start + 6 > runs.size())
                return false;
            Match s = matchSymbol(&runs[start], 6, kStartA, kStartC);
            if (s.value < 0)
                return false;

            vector<int> vals{ s.value };
            size_t pos = start + 6;
            bool foundStop = false;
            while (!foundStop) {
                if (pos + 7 > runs.size() || vals.size() > 250)
                    return false;
                Match st = matchSymbol(&runs[pos], 7, kStop, kStop);
                Match d = matchSymbol(&runs[pos], 6, 0, 105);
                if (st.value >= 0 && (d.value < 0 || st.err < d.err)) {
                    foundStop = true;
                }
                else {
                    if (d.value < 0)
                        return false;
                    vals.push_back(d.value);
                    pos += 6;
                }
            }

            if (vals.size() < 3)
                return false;
            long sum = vals[0];
            for (size_t i = 1; i + 1 < vals.size(); ++i)
                sum += static_cast<long>(i) * vals[i];
            if (sum % 103 != vals.back())
                return false;
            return valuesToText(vals, out);
        }

        inline vector<bool> binarizeLocal(const vector<uint8_t>& row)
        {
            const int n = static_cast<int>(row.size());
            vector<long> prefix(n + 1, 0);
            for (int i = 0; i < n; ++i)
                prefix[i + 1] = prefix[i] + row[i];
            const int half = max(10, n / 12);
            vector<bool> dark(n);
            for (int i = 0; i < n; ++i) {
                int a = max(0, i - half), b = min(n, i + half + 1);
                double mean = double(prefix[b] - prefix[a]) / (b - a);
                dark[i] = row[i] < mean * 0.92;
            }
            return dark;
        }

        inline vector<bool> binarizeGlobal(const vector<uint8_t>& row)
        {
            long hist[256] = { 0 };
            for (uint8_t v : row)
                ++hist[v];
            const long total = static_cast<long>(row.size());
            double sumAll = 0;
            for (int i = 0; i < 256; ++i)
                sumAll += i * double(hist[i]);
            double sumB = 0, best = -1;
            long darkCount = 0;
            int thr = 128;
            for (int t = 0; t < 256; ++t) {
                darkCount += hist[t];
                if (darkCount == 0) continue;
                long lightCount = total - darkCount;
                if (lightCount == 0) break;
                sumB += t * double(hist[t]);
                double darkMean = sumB / darkCount, lightMean = (sumAll - sumB) / lightCount;
                double between = double(darkCount) * double(lightCount) * (darkMean - lightMean) * (darkMean - lightMean);
                if (between > best) {
                    best = between;
                    thr = t;
                }
            }
            vector<bool> dark(row.size());
            for (size_t i = 0; i < row.size(); ++i)
                dark[i] = row[i] <= thr;
            return dark;
        }

        inline vector<int> toRuns(const vector<bool>& dark)
        {
            vector<int> runs;
            size_t i = 0;
            while (i < dark.size() && !dark[i])
                ++i;
            while (i < dark.size()) {
                const bool color = dark[i];
                size_t j = i;
                while (j < dark.size() && dark[j] == color)
                    ++j;
                runs.push_back(static_cast<int>(j - i));
                i = j;
            }
            return runs;
        }

        inline bool decodeRow(vector<uint8_t> row, string& out)
        {
            if (row.size() < 40)
                return false;
            for (int rev = 0; rev < 2; ++rev) {
                if (rev)
                    reverse(row.begin(), row.end());

                vector<uint8_t> sm(row.size());
                sm.front() = row.front();
                sm.back() = row.back();
                for (size_t i = 1; i + 1 < row.size(); ++i)
                    sm[i] = static_cast<uint8_t>((row[i - 1] + 2 * row[i] + row[i + 1]) / 4);

                for (int mode = 0; mode < 2; ++mode) {
                    vector<bool> dark;
                    if (mode == 0)
                        dark = binarizeGlobal(sm);
                    else
                        dark = binarizeLocal(sm);
                    auto runs = toRuns(dark);
                    for (size_t i = 0; i + 6 <= runs.size(); i += 2)
                        if (decodeFromRuns(runs, i, out))
                            return true;
                }
            }
            return false;
        }

    }

    inline optional<string> decodeGray(const uint8_t* data, int w, int h, int stride)
    {
        if (!data || w < 40 || h < 1)
            return nullopt;
        const int kLines = 40;
        string text;

        for (int k = 0; k < kLines; ++k) {
            int direction;
            if (k & 1)
                direction = 1;
            else
                direction = -1;
            int off = ((k + 1) / 2) * direction;
            int y = clamp(h / 2 + off * h / kLines, 0, h - 1);
            vector<uint8_t> row(data + static_cast<size_t>(y) * stride,
                data + static_cast<size_t>(y) * stride + w);
            if (detail::decodeRow(move(row), text))
                return text;
        }
        for (int k = 0; k < kLines; ++k) {
            int direction;
            if (k & 1)
                direction = 1;
            else
                direction = -1;
            int off = ((k + 1) / 2) * direction;
            int x = clamp(w / 2 + off * w / kLines, 0, w - 1);
            vector<uint8_t> col(h);
            for (int y = 0; y < h; ++y)
                col[y] = data[static_cast<size_t>(y) * stride + x];
            if (detail::decodeRow(move(col), text))
                return text;
        }
        return nullopt;
    }

}