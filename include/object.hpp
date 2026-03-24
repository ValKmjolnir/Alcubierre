#pragma once

class object {
protected:
    bool has_bloom_;

public:
    object(): has_bloom_(false) {}
    object(bool hb): has_bloom_(hb) {}
    virtual ~object() = default;
    bool has_bloom() const { return has_bloom_; }
};