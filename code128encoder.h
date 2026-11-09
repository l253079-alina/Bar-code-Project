#pragma once

#include "code128patterns.h"
#include <string>
#include <vector>
using namespace std;

namespace code128 {

    inline vector<bool> encodeDigits(const string& digits) {

        if (digits.empty() || digits.size() % 2 != 0 || digits[0] == '0') {
            return {};
        }

        for (int i = 0; i < digits.size(); i++) {
            if (digits[i] < '0' || digits[i] > '9') {
                return {};
            }
        }

        vector<int> values{ kStartC };
        for (size_t i = 0; i < digits.size(); i += 2) {
            values.push_back((digits[i] - '0') * 10 + (digits[i + 1] - '0'));
        }

        long sum = values[0];
        for (size_t i = 1; i < values.size(); ++i) {
            sum += static_cast<long>(i) * values[i];
        }
        values.push_back(static_cast<int>(sum % 103));
        values.push_back(kStop);

        vector<bool> modules(kQuietModules, false);

        for (int i = 0; i < values.size(); i++) {
            int v = values[i];
            bool dark = true;
            for (const char* p = kPatterns[v]; *p; ++p) {
                modules.insert(modules.end(), *p - '0', dark);
                dark = !dark;
            }
        }
        modules.insert(modules.end(), kQuietModules, false);
        return modules;
    }

}