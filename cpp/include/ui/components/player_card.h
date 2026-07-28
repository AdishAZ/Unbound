#pragma once

#include "ui/widget.h"
#include <string>
#include <cstdint>

namespace unboundmp::ui {

class PlayerCard : public Widget {
public:
    enum class Status { Online, Offline, Away };

    PlayerCard();
    
    void SetPlayerData(uint64_t id, const std::string& name, Status status);
    
    void Render(const RenderContext& ctx) override;

private:
    uint64_t id_ = 0;
    std::string name_;
    Status status_ = Status::Offline;
};

} // namespace unboundmp::ui
