#include "UMGHierarchyContextMenuExtension.h"

void FUMGHierarchyContextMenuExtension::ExtendContextMenu(
	FMenuBuilder& MenuBuilder,
	TSharedRef<FWidgetBlueprintEditor> BlueprintEditor,
	FVector2D TargetLocation
) const
{
	MenuBuilder.AddMenuSeparator();

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("自定义功能")),
		FText::FromString(TEXT("Run my custom logic")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([BlueprintEditor]()
			{
				TSet<TWeakObjectPtr<UObject>> SelectedObjects = BlueprintEditor->GetSelectedObjects();
				TSet<FWidgetReference> SelectedWidgets = BlueprintEditor->GetSelectedWidgets();
				for (TWeakObjectPtr<UObject> Object : SelectedObjects)
				{
					if (!Object.IsValid()) continue;
					UE_LOG(LogTemp, Log, TEXT("[Menu] Object: %s"), *Object->GetName());
				}
				for (FWidgetReference Widget : SelectedWidgets)
				{
					if (!Widget.GetTemplate()) continue;
					UE_LOG(LogTemp, Log, TEXT("[Menu] Widget: %s"), *Widget.GetTemplate()->GetName());
				}
			}))
	);
}
