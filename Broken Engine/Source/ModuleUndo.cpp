#include "ModuleUndo.h"
#include "mmgr/mmgr.h"

using namespace Broken;

ModuleUndo::ModuleUndo(bool start_enabled) : Module(start_enabled) {
	name = "Undo";
}

ModuleUndo::~ModuleUndo() {
}

const Broken::json& ModuleUndo::SaveStatus() const {
	static Broken::json config;
	config["stackSize"] = stackSize;
	return config;
}

void ModuleUndo::LoadStatus(const Broken::json& file) {
	if (!file[name].is_null())
		stackSize = file[name]["stackSize"];
}

void ModuleUndo::pushCmd(UndoCommand* cmd) {
	bool merged = false;
	if (cmd->label() != nullptr && cmd->label() == undoStack[undoIndex]->label())
		merged = undoStack[undoIndex]->mergeWith(cmd);

	// If it has been merged cmd is unneeded
	if (merged)
		delete cmd;

	else {
		if (++undoIndex == stackSize)
			undoIndex = 0;

		// Older commands are eliminated
		if (undoStack[undoIndex] != nullptr)
			delete undoStack[undoIndex];

		undoStack[undoIndex] = cmd;
	}

}

void ModuleUndo::undo() {
	UndoCommand* undo_cmd = undoStack[undoIndex];
	if (undo_cmd != nullptr) {
		undo_cmd->undo();
		if (undo_cmd->isObsolete())
			delete undo_cmd;
		else {
			if (++redoIndex == stackSize)
				redoIndex = 0;

			// Older commands are eliminated
			if (redoStack[redoIndex] != nullptr)
				delete redoStack[redoIndex];

			redoStack[redoIndex] = undo_cmd;
		}

		undoStack[undoIndex] = nullptr;
		if (--undoIndex == -1)
			undoIndex += stackSize;
	}
}

void ModuleUndo::redo() {
	UndoCommand* redo_cmd = redoStack[redoIndex];
	if (redo_cmd != nullptr) {
		redo_cmd->redo();
		if (redo_cmd->isObsolete())
			delete redo_cmd;
		else {
			if (++undoIndex == stackSize)
				undoIndex = 0;

			// Older commands are eliminated
			if (undoStack[undoIndex] != nullptr)
				delete undoStack[undoIndex];

			undoStack[undoIndex] = redo_cmd;
		}

		redoStack[redoIndex] = nullptr;
		if (--redoIndex == -1)
			redoIndex += stackSize;
	}
}

void ModuleUndo::clearStacks() {
	for (int i = 0; i < stackSize; ++i) {
		if (undoStack[i] != nullptr) {
			delete undoStack[i];
			undoStack[i] = nullptr;
		}
		if (redoStack[i] != nullptr) {
			delete redoStack[i];
			redoStack[i] = nullptr;
		}
	}
}

void ModuleUndo::changeStackSize(uint new_size) {
	// We recreate the stacks
	UndoCommand** newUndo = new UndoCommand*[new_size];

	uint newIndex = 0;
	for (int i = 0; i < stackSize && undoStack[undoIndex] != nullptr; ++i, ++newIndex) {
		newUndo[i] = undoStack[undoIndex++];
		if (undoIndex == stackSize)
			undoIndex = 0;
	}

	for (int i = newIndex; i < new_size; ++i)
		newUndo[i] = nullptr;

	undoIndex = newIndex - 1;
	delete undoStack;
	undoStack = newUndo;

	// We recreate the stacks
	UndoCommand** newRedo = new UndoCommand * [new_size];

	newIndex = 0;
	for (int i = 0; i < stackSize && redoStack[redoIndex] != nullptr; ++i, ++newIndex) {
		newRedo[i] = redoStack[redoIndex++];
		if (redoIndex == stackSize)
			redoIndex = 0;
	}

	for (int i = newIndex; i < new_size; ++i)
		newRedo[i] = nullptr;

	redoIndex = newIndex - 1;
	delete redoStack;
	redoStack = newUndo;

	stackSize = new_size;
}
