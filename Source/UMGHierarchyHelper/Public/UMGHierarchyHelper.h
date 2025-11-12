// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "WidgetBlueprintEditor.h"
#include "WidgetBlueprint.h"
#include "Editor/UMGEditor/Private/Hierarchy/SHierarchyViewItem.h"
#include "UMGHierarchyContextMenuExtension.h"

class FUMGHierarchyHelper : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FTSTicker::FDelegateHandle TickDelegate;
    TSharedPtr<FUMGHierarchyContextMenuExtension> HierarchyContextMenuExtension;

	bool Tick(float);
	void SetHiddenWidgets(FWidgetBlueprintEditor* Editor);
    void SetHierarchyItemColor(TSharedRef<SHierarchyViewItem> HierarchyItem, UWidget* Widget);

    // 获取当前UMG的Hierarchy中的所有项
    void GetUMGHierarchyViewWidgets(
        UWidgetBlueprint* WidgetBlueprint,
        TArray<TSharedRef<SHierarchyViewItem>>& OutItems,
        TSharedPtr<SSearchBox>& OutSearchBox);

    void GetChildren(FWidgetBlueprintEditor* InBlueprintEditor, TArray<UWidget*>& OutChildren);
    void GetChildren(UWidgetBlueprint* Blueprint, FName InSlotName, TArray<UWidget*>& OutChildren);
    void GetChildren(UWidget* InWidget, FName InSlotName, TArray<UWidget*>& OutChildren);
    void GetChildren(UWidget* InWidget, TArray<UWidget*>& OutChildren);

    // 递归查找第一个指定类型的子 Widget
    template<typename WidgetType>
    TSharedPtr<WidgetType> FindChildrenOfType(const TSharedRef<SWidget>& ParentWidget, FString TypeName = FString())
    {
        // 获取当前 Widget 的子节点
        FChildren* Children = ParentWidget->GetChildren();
        if (!Children)
            return nullptr;

        if (TypeName.IsEmpty())
        {
            // 获取 WidgetType 的类型名称
            TypeName = WidgetType::StaticWidgetClass().GetWidgetType().ToString();
        }

        for (int32 i = 0; i < Children->Num(); ++i)
        {
            TSharedRef<SWidget> Child = Children->GetChildAt(i);

            // 如果是指定类型，则返回
            if (Child->GetTypeAsString() == TypeName)
            {
                // 用 StaticCast 确保类型安全
                return StaticCastSharedRef<WidgetType>(Child);
            }

            // 递归遍历子节点
            TSharedPtr<WidgetType> Result = FindChildrenOfType<WidgetType>(Child, TypeName);
            if (Result.IsValid())
            {
                return Result;
            }
        }

        return nullptr;
    }

    // 递归查找所有指定类型的子 Widget
    template<typename WidgetType>
    void FindAllChildrenOfType(const TSharedRef<SWidget>& ParentWidget, TArray<TSharedRef<WidgetType>>& OutWidgets, FString TypeName = FString())
    {
        // 获取当前 Widget 的子节点
        FChildren* Children = ParentWidget->GetChildren();
        if (!Children)
            return;

        if (TypeName.IsEmpty())
        {
            // 获取 WidgetType 的类型名称
            TypeName = WidgetType::StaticWidgetClass().GetWidgetType().ToString();
        }

        for (int32 i = 0; i < Children->Num(); ++i)
        {
            TSharedRef<SWidget> Child = Children->GetChildAt(i);

            // 如果是指定类型，则加入结果
            if (Child->GetTypeAsString() == TypeName)
            {
                // 用 StaticCast 确保类型安全
                OutWidgets.Add(StaticCastSharedRef<WidgetType>(Child));
            }

            // 递归遍历子节点
            FindAllChildrenOfType<WidgetType>(Child, OutWidgets, TypeName);
        }
    }
};
