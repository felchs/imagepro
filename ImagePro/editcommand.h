#pragma once

#include <vector>
#include <map>
#include <string>
#include <any>
#include <memory>

#include "render.h"

namespace imagepro
{
	using namespace std;

	class CRender;

	struct CommandList
	{
		map<string, std::any> commands;
	};

	class EditCommand
	{
	public:
		void Undo()
		{
			if (CanUndo())
			{
				currIdx++;
				ApplyCommand(commandList[currIdx]);
			}
		}

		void Redo()
		{
			if (CanRedo())
			{
				currIdx--;
				ApplyCommand(commandList[currIdx]);
			}
		}

		bool CanUndo()
		{
			int sz = commandList.size();
			if (sz == 0)
			{
				return false;
			}

			return currIdx + 1 < sz;
		}

		bool CanRedo()
		{
			int sz = commandList.size();
			if (sz == 0)
			{
				return false;
			}

			return currIdx > 0;
		}

		virtual void AddCommand() = 0;

	protected:
		void AddInternalCommand(shared_ptr<CommandList> command)
		{
			commandList.push_back(command);
		}

		virtual void ApplyCommand(shared_ptr<CommandList> command) = 0;

	private:
		vector<shared_ptr<CommandList>> commandList;

		// idx 0 is the latest
		int currIdx = 0;
	};

	class ImageProEditCommand : public EditCommand
	{
	public:
		ImageProEditCommand(CRender* render) : render(render)
		{
		}

		void ApplyCommand(shared_ptr<CommandList> command) override
		{
			// Implementation for applying a command
		}

		void AddCommand() override
		{
			// Implementation for adding a command
		}

	private:
		CRender* render;
	};
}
