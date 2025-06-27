//
// Created by cyberarm on 2025-06-27.
//

#pragma once

#include "w3d_file.h"

struct W3dPivot {
    W3dPivotStruct data() { return m_data; }
    bool is_proxy() { return m_proxy; }
    void set_proxy(bool proxy) { m_proxy = proxy; }

    W3dPivotStruct m_data;
    bool m_proxy = false;
};
