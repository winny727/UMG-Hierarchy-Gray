#pragma once
#include "WidgetBlueprintEditor.h"
#include "IHasWidgetContextMenuExtensibility.h"

class FUMGHierarchyContextMenuExtension : public IWidgetContextMenuExtension
{
public:
	virtual void ExtendContextMenu(
		FMenuBuilder& MenuBuilder,
		TSharedRef<FWidgetBlueprintEditor> BlueprintEditor,
		FVector2D TargetLocation
	) const override;
};
