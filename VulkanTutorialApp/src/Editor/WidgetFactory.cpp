#include "WidgetFactory.h"
#include "VTA_Widget.h"

namespace VTA_UI
{
    // creates a shared pointer and saves a copy of it within the registry map and top level widgets

    std::shared_ptr<VTAWidget> WidgetFactory::create(WidgetTypes type)
    {
        
        
        auto sharedPointer = std::make_shared<VTAWidget>();
        topLevelWidgets.push_back(sharedPointer);

        sharedPointer->isTopLevel = true;
        sharedPointer->topLevelIterator = std::prev(topLevelWidgets.end());
        return sharedPointer;
    }

    std::shared_ptr<VTAWidget> WidgetFactory::create(WidgetTypes type, std::shared_ptr<VTAWidget> parent)
    {
        
        auto sharedPointer = std::make_shared<VTAWidget>();

        parent->AddChild(sharedPointer);
        sharedPointer->isTopLevel = false;
        return sharedPointer;
    }
    
    
    void WidgetFactory::destroy(std::shared_ptr<VTAWidget> widget)   
    {
        if (widget.use_count() > 1)
        {
            throw std::runtime_error("other shared pointers exist to this object");
        }

        for (std::shared_ptr<VTAWidget>& child : widget->children)
        {
            VTAWidget::destroy(std::move(child));
        }
        
        if (widget->isTopLevel)
        {
            topLevelWidgets.erase(widget->topLevelIterator);
        }
    }


}
