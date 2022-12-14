#include "BoundImpTreeModel.h"

//-----------------------------------------------------------------------------

#define NOT_FILLED  "-"

int BoundImpTreeModel::columnCount(const QModelIndex &parent) const
{
	ExeNodeWrapper* impWrap = dynamic_cast<ExeNodeWrapper*>(wrapper());
	if (!impWrap) return 0;
	ExeNodeWrapper* entry = impWrap->getEntryAt(0);
	if (!entry) return 0;
	uint32_t cntr = entry->getFieldsCount() + ADDED_COLS_NUM;
	return cntr;
}


QVariant BoundImpTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole) return QVariant();
	switch (section) {
		case OFFSET: return "Offset";
		case NAME: return "Name";
	}
	ExeNodeWrapper* impWrap = dynamic_cast<ExeNodeWrapper*>(wrapper());
	if (!impWrap) return QVariant();

	int32_t fID = this->columnToFID(section);
	return impWrap->getSubfieldName(0, fID);
}

QVariant BoundImpTreeModel::data(const QModelIndex &index, int role) const
{
	BoundEntryWrapper* wrap = dynamic_cast<BoundEntryWrapper*>(wrapperAt(index));
	if (!wrap) return QVariant();

	int column = index.column();
	if (role == Qt::ForegroundRole) return this->addrColor(index);
	if (role == Qt::FontRole) {
		if (this->containsOffset(index) || this->containsValue(index)) return offsetFont;
	}
	if (role == Qt::ToolTipRole) return toolTip(index);

	if (role != Qt::DisplayRole && role != Qt::EditRole) return QVariant();
	int fId = getFID(index);
	switch (column) {
		case OFFSET: return QString::number(getFieldOffset(index), 16);
		case NAME: return wrap->getName();
	}
	return dataValue(index);
}

bool BoundImpTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid()) return false;

	size_t fId = index.row();
	uint32_t sID = this->columnToFID(index.column());

	if (!wrapper()) return false;
	DelayImpEntryWrapper* entry =  dynamic_cast<DelayImpEntryWrapper*>(wrapperAt(index));
	//ExeElementWrapper *entry = wrapperAt(index);
	if (!entry) return false;

	QString text = value.toString();

	bool isModified = false;
	uint32_t offset = 0;
	uint32_t fieldSize = 0;

	if (index.column() == NAME) {
		char* textPtr = entry->getLibraryName();
		if (!textPtr) return false;

		offset = entry->getOffset(textPtr);
		fieldSize = text.size() + 2;

		this->myPeHndl->backupModification(offset, fieldSize);
		isModified = m_PE->setTextValue(textPtr, text.toStdString(), fieldSize);

	} else {
		
		bool isOk = false;
		ULONGLONG number = text.toLongLong(&isOk, 16);
		if (!isOk) return false;

		offset = entry->getFieldOffset(sID);
		fieldSize = entry->getFieldSize(sID);

		this->myPeHndl->backupModification(offset, fieldSize);
		isModified = entry->setNumValue(sID, index.column(), number);
	}

	if (isModified) {
		this->myPeHndl->setBlockModified(offset, fieldSize);
		return true;
	}
	this->myPeHndl->unbackupLastModification();
	return false;
}
//----------------------------------------------------------------------------
