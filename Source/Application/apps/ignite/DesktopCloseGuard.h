// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <functional>
#include <memory>

class DesktopCloseGuard
{
public:
    enum Choice {cancel=0,save=1,discard=2};
    using Answer=std::function<void(int)>;
    DesktopCloseGuard(std::function<bool()> dirty,std::function<bool()> write,std::function<void()> close,
                      std::function<void(Answer)> ask,std::function<void()> failed)
        : state(std::make_shared<State>(State{false,std::move(dirty),std::move(write),std::move(close),std::move(ask),std::move(failed)})) {}
    void request()
    {
        auto current=state;
        if(current->pending) return;
        if(!current->dirty()) {current->close();return;}
        current->pending=true;
        current->ask([weak=std::weak_ptr<State>(current)](int choice)
        {
            auto active=weak.lock(); if(!active || !active->pending) return;
            active->pending=false;
            if(choice==discard) active->close();
            else if(choice==save)
            {
                if(active->write()) active->close(); else active->failed();
            }
        });
    }
private:
    struct State
    {
        bool pending;
        std::function<bool()> dirty,write;
        std::function<void()> close;
        std::function<void(Answer)> ask;
        std::function<void()> failed;
    };
    std::shared_ptr<State> state;
};
