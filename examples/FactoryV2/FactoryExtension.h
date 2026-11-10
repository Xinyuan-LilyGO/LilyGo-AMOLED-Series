#pragma once

#include "FactoryData.h"

class LilyGo_Class;
struct __BoardsConfigure;
typedef struct __BoardsConfigure BoardsConfigure_t;

class FactoryExtension {
public:
    virtual ~FactoryExtension() = default;
    /* Stable machine id is used for logging and future persistence. */
    virtual const char *id() const = 0;
    /* Human-facing metadata keeps the UI independent from the implementation. */
    virtual const char *title() const = 0;
    virtual const char *icon() const = 0;
    virtual bool begin(LilyGo_Class &board, const BoardsConfigure_t *configuration) = 0;
    virtual void update(FactoryExtensionStatus &status) = 0;
    virtual void action(uint8_t action) { (void)action; }
};
