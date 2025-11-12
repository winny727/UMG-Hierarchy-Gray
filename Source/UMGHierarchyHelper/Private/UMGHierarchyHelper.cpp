// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGHierarchyHelper.h"
#include "WidgetDetailCustomization.h"
#include "UMGEditorModule.h"

#include "WidgetBlueprintEditor.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Widgets/Input/SSearchBox.h"

#include "Editor/UMGEditor/Private/Hierarchy/SHierarchyView.h"
#include "Editor/UMGEditor/Private/Hierarchy/SHierarchyViewItem.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

// UMG Hierarchy实现类似Unity/Cocos中物体隐藏后Hierarchy上的标签置灰的效果
// 适用于 UE 5.6
// 性能消耗待测试

void FUMGHierarchyHelper::StartupModule()
{
	TickDelegate = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateRaw(this, &FUMGHierarchyHelper::Tick), 0.5f);

    // UMG Hierarchy右键菜单注册（未写具体功能，可以根据需要加）
    //IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>("UMGEditor");
    //HierarchyContextMenuExtension = MakeShared<FUMGHierarchyContextMenuExtension>();
    //UMGEditorModule.GetWidgetContextMenuExtensibilityManager()->AddExtension(HierarchyContextMenuExtension.ToSharedRef());

    // Detail面板扩展，使UWidget显示Visibility在最前面
    //FPropertyEditorModule& PropertyModule =
    //    FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    //PropertyModule.RegisterCustomClassLayout(
    //    UWidget::StaticClass()->GetFName(),
    //    FOnGetDetailCustomizationInstance::CreateStatic(&UWidgetDetailCustomization::MakeInstance)
    //);

    //for (TObjectIterator<UClass> It; It; ++It)
    //{
    //    UClass* Class = *It;
    //    if (Class->IsChildOf(UWidget::StaticClass()) && !Class->HasAnyClassFlags(CLASS_Abstract))
    //    {
    //        PropertyModule.RegisterCustomClassLayout(
    //            Class->GetFName(),
    //            FOnGetDetailCustomizationInstance::CreateStatic(&UWidgetDetailCustomization::MakeInstance)
    //        );
    //    }
    //}

    //PropertyModule.NotifyCustomizationModuleChanged();
}

void FUMGHierarchyHelper::ShutdownModule()
{
	FTSTicker::GetCoreTicker().RemoveTicker(TickDelegate);

    //if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
    //{
    //    FPropertyEditorModule& PropertyModule =
    //        FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    //    PropertyModule.UnregisterCustomClassLayout(UWidget::StaticClass()->GetFName());
    //}

    //if (FModuleManager::Get().IsModuleLoaded("UMGEditor"))
    //{
    //    IUMGEditorModule& UMGEditorModule = FModuleManager::GetModuleChecked<IUMGEditorModule>("UMGEditor");
    //    UMGEditorModule.GetWidgetContextMenuExtensibilityManager()->RemoveExtension(HierarchyContextMenuExtension.ToSharedRef());
    //}
}

bool FUMGHierarchyHelper::Tick(float)
{
    TArray<IAssetEditorInstance*> OpenEditors =
        GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllOpenEditors();

    for (IAssetEditorInstance* Instance : OpenEditors)
    {
        if (auto* BP_Editor = static_cast<FWidgetBlueprintEditor*>(Instance))
        {
            SetHiddenWidgets(BP_Editor);
        }
    }
    return true;
}

void FUMGHierarchyHelper::SetHiddenWidgets(FWidgetBlueprintEditor* Editor)
{
    UWidgetBlueprint* WidgetBP = Editor->GetWidgetBlueprintObj();
    if (!WidgetBP) return;

    UWidgetTree* WidgetTree = WidgetBP->WidgetTree;
    if (!WidgetTree) return;

    // 根据Slate对象提供的GetContent和GetTypeAsString接口，递归遍历Children获取到指定类型
    TArray<TSharedRef<SHierarchyViewItem>> HierarchyItems;
    TSharedPtr<SSearchBox> SearchBox;
    GetUMGHierarchyViewWidgets(WidgetBP, HierarchyItems, SearchBox);

    // NOTE: GetLabelTextWithMetadata也可以用来匹配，但是会有内容重复的问题，
    // 总之貌似没办法在拿到SHierarchyViewItem后很方便的跟其对应的UWidget关联上

    // 根据SHierarchyViewItem里面的规则，实现相同的获取逻辑，使UWidget和HierarchyItems能对应上
    TArray<UWidget*> Children;
    GetChildren(Editor, Children);

    if (SearchBox.IsValid() && !SearchBox->GetText().IsEmpty())
    {
        //简单处理，搜索框不为空时就不置灰了
        return;
    }

    if (Children.Num() != HierarchyItems.Num())
    {
        //UE_LOG(LogTemp, Error, TEXT("HierarchyItems Num Match Error"));
        return;
    }

    for (int32 i = 0; i < Children.Num(); i++)
    {
        UWidget* Widget = Children[i];
        TSharedRef<SHierarchyViewItem> HierarchyItem = HierarchyItems[i];
        SetHierarchyItemColor(HierarchyItem, Widget);
    }
}

void FUMGHierarchyHelper::SetHierarchyItemColor(TSharedRef<SHierarchyViewItem> HierarchyItem, UWidget* Widget)
{
    if (!Widget) return;

    TSharedPtr<SInlineEditableTextBlock> TextBlock = FindChildrenOfType<SInlineEditableTextBlock>(HierarchyItem, "SInlineEditableTextBlock");
    if (!TextBlock.IsValid()) return;

    // 改变文字颜色
    const bool bVisible = (Widget->GetVisibility() != ESlateVisibility::Hidden); // 这里只处理了Hidden
    FLinearColor Color = bVisible ? FLinearColor::White : FLinearColor(0.3f, 0.3f, 0.3f);
    TextBlock->SetColorAndOpacity(Color);

    /*UE_LOG(LogTemp, Log, TEXT("SetHierarchyItemColor Widget: %s, HierarchyItem: %s"), *Widget->GetLabelTextWithMetadata().ToString(), *TextBlock->GetText().ToString());*/
}

void FUMGHierarchyHelper::GetUMGHierarchyViewWidgets(
    UWidgetBlueprint* WidgetBlueprint,
    TArray<TSharedRef<SHierarchyViewItem>>& OutItems,
    TSharedPtr<SSearchBox>& OutSearchBox)
{
    if (!WidgetBlueprint) return;

    // 获取当前激活的 UMG Blueprint 编辑器窗口
    FWidgetBlueprintEditor* ActiveEditor = nullptr;
    {
        TArray<IAssetEditorInstance*> Editors =
            GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->GetAllOpenEditors();

        for (IAssetEditorInstance* Inst : Editors)
        {
            if (auto* WBE = static_cast<FWidgetBlueprintEditor*>(Inst))
            {
                if (WBE->GetWidgetBlueprintObj() == WidgetBlueprint)
                {
                    ActiveEditor = WBE;
                    break;
                }
            }
        }
    }
    if (!ActiveEditor) return;

    TSharedPtr<SDockTab> HierarchyTab = ActiveEditor->GetTabManager()->FindExistingLiveTab(FName("SlateHierarchy"));
    if (HierarchyTab.IsValid())
    {
        TSharedPtr<SWidget> TabContent = HierarchyTab->GetContent();
        if (auto* HierarchyView = static_cast<SHierarchyView*>(TabContent.Get()))
        {
            TSharedRef<SWidget> SharedHierarchyView = HierarchyView->AsShared();
            FindAllChildrenOfType<SHierarchyViewItem>(SharedHierarchyView, OutItems, "SHierarchyViewItem");
            OutSearchBox = FindChildrenOfType<SSearchBox>(SharedHierarchyView, "SSearchBox");
        }
    }

    return;
}

// FHierarchyRoot
void FUMGHierarchyHelper::GetChildren(FWidgetBlueprintEditor* InBlueprintEditor, TArray<UWidget*>& OutChildren)
{
    if (!InBlueprintEditor) return;

    UWidgetBlueprint* Blueprint = InBlueprintEditor->GetWidgetBlueprintObj();
    UUserWidget* UserWidget = Blueprint->GeneratedClass->GetDefaultObject<UUserWidget>();
    OutChildren.Add(UserWidget);

    UWidget* RootChild = Blueprint->WidgetTree->RootWidget;
    GetChildren(RootChild, OutChildren);

    TSet<FName> InheritedNamedSlotsWithContentInSameTree = Blueprint->GetInheritedNamedSlotsWithContentInSameTree();
    for (const FName& SlotName : Blueprint->GetInheritedAvailableNamedSlots())
    {
        if (InheritedNamedSlotsWithContentInSameTree.Contains(SlotName))
        {
            if (!Blueprint->WidgetTree->GetContentForSlot(SlotName))
            {
                continue;
            }
        }

        GetChildren(Blueprint, SlotName, OutChildren);
    }
}

// FNamedSlotModelSubclass
void FUMGHierarchyHelper::GetChildren(UWidgetBlueprint* Blueprint, FName InSlotName, TArray<UWidget*>& OutChildren)
{
    if (!Blueprint) return;

    if (INamedSlotInterface* NamedSlotHost = Blueprint->WidgetTree)
    {
        if (UWidget* SlotContent = NamedSlotHost->GetContentForSlot(InSlotName))
        {
            OutChildren.Add(SlotContent);
            GetChildren(SlotContent, OutChildren);
        }
    }
}

// FNamedSlotModel
void FUMGHierarchyHelper::GetChildren(UWidget* InWidget, FName InSlotName, TArray<UWidget*>& OutChildren)
{
    if (!InWidget) return;

    OutChildren.Add(InWidget);

    if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(InWidget))
    {
        if (UWidget* SlotContent = NamedSlotHost->GetContentForSlot(InSlotName))
        {
            GetChildren(SlotContent, OutChildren);
        }
    }
}

// FHierarchyWidget
void FUMGHierarchyHelper::GetChildren(UWidget* InWidget, TArray<UWidget*>& OutChildren)
{
    if (!InWidget) return;

    OutChildren.Add(InWidget);

    // Check for named slots
    if (INamedSlotInterface* NamedSlotHost = Cast<INamedSlotInterface>(InWidget))
    {
        TArray<FName> SlotNames;
        NamedSlotHost->GetSlotNames(SlotNames);

        for (FName& SlotName : SlotNames)
        {
            GetChildren(InWidget, SlotName, OutChildren);
        }
    }

    // Check if it's a panel widget that can support children
    if (UPanelWidget* PanelWidget = Cast<UPanelWidget>(InWidget))
    {
        for (int32 i = 0; i < PanelWidget->GetChildrenCount(); i++)
        {
            UWidget* Child = PanelWidget->GetChildAt(i);
            if (Child)
            {
                GetChildren(Child, OutChildren);
            }
        }
    }
}

IMPLEMENT_MODULE(FUMGHierarchyHelper, UMGHierarchyHelper)