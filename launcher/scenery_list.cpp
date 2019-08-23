#include "stdafx.h"
#include "scenery_list.h"
#include "imgui/imgui.h"
#include "utilities.h"
#include "renderer.h"
#include "McZapkie/MOVER.h"
#include "application.h"
#include "Logs.h"
#include <filesystem>

ui::scenerylist_panel::scenerylist_panel(scenery_scanner &scanner)
    : ui_panel(STR("Scenario list"), false), scanner(scanner), placeholder_mini("textures/mini/other")
{
}

void ui::scenerylist_panel::render()
{
	if (!is_open)
		return;

	auto const panelname{(title.empty() ? m_name : title) + "###" + m_name};

	if (ImGui::Begin(panelname.c_str(), &is_open)) {
		ImGui::Columns(3);

		if (ImGui::BeginChild("child1", ImVec2(0, -200))) {
			std::string prev_prefix;
			bool collapse_open = false;

			for (auto &desc : scanner.scenarios) {
				std::string name = desc.path.stem().string();
				std::string prefix = name.substr(0, name.find_first_of("-_"));
				if (prefix.empty())
					prefix = name;

				bool just_opened = false;
				if (prefix != prev_prefix) {
					collapse_open = ImGui::CollapsingHeader(prefix.c_str());
					just_opened = ImGui::IsItemDeactivated();
					prev_prefix = prefix;
				}

				if (collapse_open) {
					if (just_opened)
						selected_scenery = &desc;

					ImGui::Indent(10.0f);
					if (ImGui::Selectable(name.c_str(), &desc == selected_scenery)) {
						selected_scenery = &desc;
						selected_trainset = nullptr;
					}
					ImGui::Unindent(10.0f);
				}
			}
		} ImGui::EndChild();

		ImGui::NextColumn();

		if (ImGui::BeginChild("child2", ImVec2(0, -200))) {
			if (selected_scenery) {
				ImGui::TextWrapped("%s", selected_scenery->name.c_str());
				ImGui::TextWrapped("%s", selected_scenery->description.c_str());

				ImGui::Separator();

				for (auto const &link : selected_scenery->links) {
					if (ImGui::Button(link.second.c_str(), ImVec2(-1, 0))) {
						std::string file = ToLower(link.first);
#ifdef _WIN32
						system(("start \"" + file + "\"").c_str());
#elif __linux__
						system(("xdg-open \"" + file + "\"").c_str());
#elif __APPLE__
						system(("open \"" + file + "\"").c_str());
#endif
					}
				}
			}
		} ImGui::EndChild();

		ImGui::NextColumn();

		if (selected_scenery) {
			if (ImGui::BeginChild("child3", ImVec2(0, -200), false, ImGuiWindowFlags_NoScrollbar)) {
				if (!selected_scenery->image_path.empty()) {
					scenery_desc *desc = const_cast<scenery_desc*>(selected_scenery);
					desc->image = GfxRenderer.Fetch_Texture(selected_scenery->image_path, true);
					desc->image_path.clear();
				}

				if (selected_scenery->image != null_handle) {
					opengl_texture &tex = GfxRenderer.Texture(selected_scenery->image);
					tex.create();

					if (tex.is_ready) {
						float avail_width = ImGui::GetContentRegionAvailWidth();
						float height = avail_width / tex.width() * tex.height();

						ImGui::Image(reinterpret_cast<void *>(tex.id), ImVec2(avail_width, height), ImVec2(0, 1), ImVec2(1, 0));
					}
				}
			} ImGui::EndChild();
		}
		ImGui::Columns();
		ImGui::Separator();

		if (ImGui::BeginChild("child4")) {
			if (selected_scenery) {
				if (selected_trainset)
					ImGui::Columns(2);

				if (ImGui::BeginChild("child5", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
					ImGuiListClipper clipper(selected_scenery->trainsets.size());
					while (clipper.Step()) for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						auto &trainset = selected_scenery->trainsets[i];
						ImGui::PushID(i);
						if (ImGui::Selectable("##set", selected_trainset == &trainset, 0, ImVec2(0, 30)))
							selected_trainset = &trainset;
						ImGui::SameLine();
						draw_trainset(trainset);
						ImGui::NewLine();
						ImGui::PopID();
						//ImGui::Selectable(z.c_str(), false);
					}
				} ImGui::EndChild();

				if (selected_trainset) {
					ImGui::NextColumn();

					ImGui::TextWrapped(selected_trainset->description.c_str());

					if (ImGui::Button(STR_C("Launch"), ImVec2(-1, 0))) {
						bool found = false;
						for (auto &veh : selected_trainset->vehicles) {
							if (veh.drivertype.size() > 0 && veh.drivertype != "nobody") {
								Global.local_start_vehicle = ToLower(veh.name);
								Global.SceneryFile = selected_scenery->path;

								std::string set = "trainset ";
								set += selected_trainset->name + " ";
								set += selected_trainset->track + " ";
								set += std::to_string(selected_trainset->offset) + " ";
								set += std::to_string(selected_trainset->velocity) + " ";
								for (const auto &veh : selected_trainset->vehicles) {
									set += "node -1 0 " + veh.name + " dynamic ";
									set += veh.vehicle->path.parent_path().generic_string() + " ";
									set += veh.skin->skin + " ";
									set += veh.vehicle->path.stem().generic_string() + " ";
									set += std::to_string(veh.offset) + " " + veh.drivertype + " ";
									set += std::to_string(veh.coupling);
									if (veh.params.size() > 0)
										set += "." + veh.params;
									set += " " + std::to_string(veh.loadcount) + " ";
									if (veh.loadcount > 0)
										set += veh.loadtype + " ";
									set += "enddynamic ";
								}
								set += "endtrainset";
								WriteLog(set);

								Global.trainset_overrides.push_back(std::make_pair(selected_trainset->file_bounds.first, set));

								Application.pop_mode();
								Application.push_mode(eu07_application::mode::scenarioloader);

								found = true;
								break;
							}
						}
						if (!found)
							ImGui::OpenPopup("missing_driver");
					}

					if (ImGui::BeginPopup("missing_driver")) {
						ImGui::TextUnformatted(STR_C("Trainset not occupied"));
						if (ImGui::Button(STR_C("OK"), ImVec2(-1, 0)))
							ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
					}
				}
			}
		} ImGui::EndChild();
	}

	ImGui::End();
}

void ui::scenerylist_panel::draw_trainset(trainset_desc &trainset)
{
	static std::unordered_map<coupling, std::string> coupling_names =
	{
	    { coupling::faux, STRN("faux") },
	    { coupling::coupler, STRN("coupler") },
	    { coupling::brakehose, STRN("brake hose") },
	    { coupling::control, STRN("control") },
	    { coupling::highvoltage, STRN("high voltage") },
	    { coupling::gangway, STRN("gangway") },
	    { coupling::mainhose, STRN("main hose") },
	    { coupling::heating, STRN("heating") },
	    { coupling::permanent, STRN("permanent") },
	    { coupling::uic, STRN("uic") }
	};

	int position = 0;

	draw_droptarget(trainset, position++);
	ImGui::SameLine(15.0f);

	for (auto const &dyn_desc : trainset.vehicles) {
		deferred_image *mini = nullptr;

		if (dyn_desc.skin && dyn_desc.skin->mini.get() != -1)
			mini = &dyn_desc.skin->mini;
		else
			mini = &placeholder_mini;

		ImGui::PushID(static_cast<const void*>(&dyn_desc));

		glm::ivec2 size = mini->size();
		float width = 30.0f / size.y * size.x;
		ImGui::Image(reinterpret_cast<void*>(mini->get()), ImVec2(width, 30), ImVec2(0, 1), ImVec2(1, 0));

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			std::string name = (dyn_desc.vehicle->path.parent_path() / dyn_desc.vehicle->path.stem()).string();
			std::string skin = dyn_desc.skin->skin;
			ImGui::Text(STR_C("ID: %s"), dyn_desc.name.c_str());
			ImGui::NewLine();

			ImGui::Text(STR_C("Type: %s"), name.c_str());
			ImGui::Text(STR_C("Skin: %s"), skin.c_str());
			ImGui::NewLine();

			if (dyn_desc.drivertype != "nobody")
				ImGui::Text(STR_C("Occupied: %s"), dyn_desc.drivertype.c_str());
			if (dyn_desc.loadcount > 0)
				ImGui::Text(STR_C("Load: %s: %d"), dyn_desc.loadtype.c_str(), dyn_desc.loadcount);
			if (!dyn_desc.params.empty())
				ImGui::Text(STR_C("Parameters: %s"), dyn_desc.params.c_str());
			ImGui::NewLine();

			ImGui::TextUnformatted(STR_C("Coupling:"));

			for (int i = 1; i <= 0x100; i <<= 1) {
				bool dummy = true;

				if (dyn_desc.coupling & i) {
					std::string label = STRN("unknown");
					auto it = coupling_names.find(static_cast<coupling>(i));
					if (it != coupling_names.end())
						label = it->second;
					ImGui::Checkbox(Translations.lookup_c(label.c_str()), &dummy);
				}
			}

			ImGui::EndTooltip();
		}

		ImGui::SameLine(0, 0);
		float originX = ImGui::GetCursorPosX();

		ImGui::SameLine(originX - 7.5f);
		draw_droptarget(trainset, position++);
		ImGui::SameLine(originX);

		ImGui::PopID();
	}
}

#include "Logs.h"
void ui::scenerylist_panel::draw_droptarget(trainset_desc &trainset, int position)
{
	ImGui::Dummy(ImVec2(15, 30));
	if (ImGui::BeginDragDropTarget()) {
		const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("vehicle_pure");
		if (payload) {
			skin_set *ptr = *(reinterpret_cast<skin_set**>(payload->Data));
			std::shared_ptr<skin_set> skin;
			for (auto &s : ptr->vehicle.lock()->matching_skinsets)
				if (s.get() == ptr)
					skin = s;

			trainset.vehicles.emplace(trainset.vehicles.begin() + position);
			dynamic_desc &desc = trainset.vehicles[position];

			desc.name = skin->skin + "_" + std::to_string((int)LocalRandom(0.0, 10000000.0));
			desc.vehicle = skin->vehicle.lock();
			desc.skin = skin;

		}
		ImGui::EndDragDropTarget();
	}
}
