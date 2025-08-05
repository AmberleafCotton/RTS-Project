#include "RTS_Widget.h"

void URTS_Widget::InitializeWidget_Implementation(ARTS_Actor* InOwner)
{
	Owner = InOwner;
}

ARTS_Actor* URTS_Widget::GetModuleOwner() const
{
	return Owner;
}

