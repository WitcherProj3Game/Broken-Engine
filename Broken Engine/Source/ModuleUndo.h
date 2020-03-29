#ifndef __MODULEUNDO_H__
#define __MODULEUNDO_H__
#pragma once

#include "Module.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE

class BROKEN_API UndoCommand {
public:
	UndoCommand() {};
	virtual ~UndoCommand() {}

	virtual void undo() {}
	virtual void redo() {}
	// Will only be called if label != nullptr && command->label() == label()
	virtual bool mergeWith(const UndoCommand* command) { return false; }

	bool isObsolete() const { return _obsolete; }
	// Obsolete commands will be deleted after undo/redo
	void setObsolete(bool obsolete) { _obsolete = obsolete;};

	virtual const char* label() const { return nullptr; }

private:
	bool _obsolete = false;
};

// Example UndoCommand, though fully functional
template <class T>
class BROKEN_API ChangeVarCmd : public UndoCommand {
public:
	// The value of &variable is changed inside the creator itself so there is no need to change it outside
	ChangeVarCmd(T* variable, T new_value) : m_variable(variable), m_new_value(new_value), m_label("ChangeVar") {
		m_old_value = *variable;
		*variable = new_value;
	}

	void undo() override {
		*m_variable = m_old_value;
	}

	void redo() override {
		*m_variable = m_new_value;
	}

	bool mergeWith(const UndoCommand* command) override {
		const ChangeVarCmd* cvar_cmd = static_cast<const ChangeVarCmd*>(command);
		ret = m_variable == cvar_cmd->m_variable;
		if (ret)
			m_new_value = cvar_cmd->m_new_value;

		return ret;	
	}

	const char* label() const override {
		return m_label;
	}

private:
	const char* m_label;
	T* m_variable;
	T m_old_value;
	T m_new_value;
};


class BROKEN_API ModuleUndo : public Module {
public:
	ModuleUndo(bool start_enabled = true);
	~ModuleUndo();

	const Broken::json& SaveStatus() const override;
	void LoadStatus(const Broken::json& file) override;


	void pushCmd(UndoCommand* cmd);
	void undo();
	void redo();

	void clearStacks();
	void changeStackSize(uint new_size);

private:
	uint stackSize = 10;

	int undoIndex = 0;
	UndoCommand** undoStack;
	int redoIndex = 0;
	UndoCommand** redoStack;
};

BE_END_NAMESPACE
#endif
