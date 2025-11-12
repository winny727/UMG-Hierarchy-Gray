#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "Components/Widget.h"

class UWidgetDetailCustomization : public IDetailCustomization
{
public:
    static TSharedRef<IDetailCustomization> MakeInstance()
    {
        return MakeShared<UWidgetDetailCustomization>();
    }
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    /** 当前选中的 UWidget 对象 */
    TWeakObjectPtr<UWidget> EditingWidget;
};