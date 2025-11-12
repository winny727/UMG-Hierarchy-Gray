#include "WidgetDetailCustomization.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"

void UWidgetDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    // 获取当前选择的对象
    TArray<TWeakObjectPtr<UObject>> SelectedObjects;
    DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);

    EditingWidget = nullptr;

    for (auto& Obj : SelectedObjects)
    {
        if (UWidget* Widget = Cast<UWidget>(Obj.Get()))
        {
            EditingWidget = Widget;
            break;
        }
    }

    // 如果没有 Widget，退出
    if (!EditingWidget.IsValid())
    {
        return;
    }

    // 添加一个自定义分类（或使用默认分类）
    IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Visibility"));
    Category.SetSortOrder(0);

    TArray<TSharedPtr<FString>> VisibilityOptions;
    UEnum* VisibilityEnum = StaticEnum<ESlateVisibility>();

    for (int32 i = 0; i < VisibilityEnum->NumEnums() - 1; ++i)
    {
        FString EnumName = VisibilityEnum->GetDisplayNameTextByIndex(i).ToString();
        VisibilityOptions.Add(MakeShared<FString>(EnumName));
    }

    // 创建一个共享变量保存当前选项
    TSharedPtr<FString> CurrentSelection = MakeShared<FString>(
        VisibilityEnum->GetDisplayNameTextByValue((int64)EditingWidget->GetVisibility()).ToString()
    );

    Category.AddCustomRow(FText::FromString(TEXT("Visibility")))
        .NameContent()
        [
            SNew(STextBlock)
                .Text(FText::FromString(TEXT("Visibility")))
                .Font(IDetailLayoutBuilder::GetDetailFont())
        ]
        .ValueContent()
        .MinDesiredWidth(200.f)
        [
            SNew(SComboBox<TSharedPtr<FString>>)
                .OptionsSource(&VisibilityOptions)
                .InitiallySelectedItem(CurrentSelection)
                .OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem)
                    {
                        return SNew(STextBlock)
                            .Text(FText::FromString(*InItem))
                            .Font(IDetailLayoutBuilder::GetDetailFont());
                    })
                .OnSelectionChanged_Lambda([this, VisibilityEnum](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
                    {
                        if (!EditingWidget.IsValid() || !NewSelection.IsValid())
                        {
                            return;
                        }

                        // 找到选中的枚举值
                        for (int32 i = 0; i < VisibilityEnum->NumEnums() - 1; ++i)
                        {
                            if (VisibilityEnum->GetDisplayNameTextByIndex(i).ToString() == *NewSelection)
                            {
                                ESlateVisibility NewVis = (ESlateVisibility)VisibilityEnum->GetValueByIndex(i);
                                EditingWidget->SetVisibility(NewVis);
                                break;
                            }
                        }
                    })
                .Content()
                [
                    SNew(STextBlock)
                        .Font(IDetailLayoutBuilder::GetDetailFont())
                        .Text_Lambda([this]()
                            {
                                if (!EditingWidget.IsValid())
                                {
                                    return FText::FromString(TEXT("-"));
                                }

                                UEnum* Enum = StaticEnum<ESlateVisibility>();
                                return Enum->GetDisplayNameTextByValue((int64)EditingWidget->GetVisibility());
                            })
                ]
        ];
}
