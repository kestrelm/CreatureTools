#include <iostream>
#include <fstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <cstdio>
#include <string>
#include <vector>
#include <rapidjson/filereadstream.h>
#include <CreatureFlatData_generated.h>
#include <flatbuffers.h>
#include <ConvertFlatData.h>

// This reads in a Creature JSON File
void
ReadCreatureJson(const std::string& filename_in, rapidjson::Document& doc)
{
	FILE* fp = fopen(filename_in.c_str(), "rb"); // non-Windows use "r"
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	doc.ParseStream(is);
	fclose(fp);
}

static std::vector<float>
GetFloatArray(rapidjson::Value& array_in)
{
	std::vector<float> ret_array(array_in.Size());
	for (int i = 0; i < array_in.Size(); i++)
	{
		ret_array[i] = (float)array_in[i].GetDouble();
	}

	return ret_array;
}

static std::vector<int>
GetIntArray(rapidjson::Value& array_in)
{
	std::vector<int> ret_array(array_in.Size());
	for (int i = 0; i < array_in.Size(); i++)
	{
		ret_array[i] = (int)array_in[i].GetInt();
	}

	return ret_array;
}

// Converts an input Creature JSON into a Creature FlatData Binary file
bool ConvertToFlatData(const std::string& json_filename_in,
	const std::string& flat_filename_out)
{
	rapidjson::Document read_doc;
	ReadCreatureJson(json_filename_in, read_doc);

	if ((!read_doc.HasMember("mesh")) || (!read_doc.HasMember("skeleton"))
		|| (!read_doc.HasMember("animation")))
	{
		std::cerr << "Error: Invalid Input Creature JSON!" << std::endl;
		return false;
	}

	flatbuffers::FlatBufferBuilder fbb;

	auto& mesh_obj = read_doc["mesh"];
	auto& skeleton_obj = read_doc["skeleton"];
	auto& animation_obj = read_doc["animation"];

	// ----------- Process Mesh ----------------------

	auto& mesh_points = mesh_obj["points"];
	auto& mesh_uvs = mesh_obj["uvs"];
	auto& mesh_indices = mesh_obj["indices"];
	auto& mesh_regions = mesh_obj["regions"];

	auto read_mesh_points = GetFloatArray(mesh_points);
	auto read_uv_points = GetFloatArray(mesh_uvs);
	auto read_indices = GetIntArray(mesh_indices);

	std::vector<flatbuffers::Offset<CreatureFlatData::meshRegion> > mesh_region_list;

	{
		// Mesh Regions
		for (rapidjson::Value::MemberIterator itr = mesh_regions.MemberBegin();
		itr != mesh_regions.MemberEnd();
			++itr)
		{
			auto& curMesh = itr->value;

			// Bone Weights
			auto& read_bone_weights = curMesh["weights"];
			std::vector<flatbuffers::Offset<CreatureFlatData::meshRegionBone> > bone_weights_list;

			for (rapidjson::Value::MemberIterator w_itr = read_bone_weights.MemberBegin();
			w_itr != read_bone_weights.MemberEnd();
				++w_itr)
			{
				CreatureFlatData::meshRegionBoneBuilder flat_mesh_region_bone(fbb);
				auto write_name = fbb.CreateString(w_itr->name.GetString());

				auto& bone_weights = read_bone_weights[w_itr->name.GetString()];
				auto write_weights = fbb.CreateVector(GetFloatArray(bone_weights));

				flat_mesh_region_bone.add_name(write_name);
				flat_mesh_region_bone.add_weights(write_weights);

				bone_weights_list.push_back(flat_mesh_region_bone.Finish());
			}

			CreatureFlatData::meshRegionBuilder flat_mesh_region(fbb);
			auto write_mesh_region_name = fbb.CreateString(itr->name.GetString());
			auto write_mesh_region_weights_list = fbb.CreateVector(bone_weights_list);

			flat_mesh_region.add_name(write_mesh_region_name);
			flat_mesh_region.add_start_pt_index(curMesh["start_pt_index"].GetInt());
			flat_mesh_region.add_end_pt_index(curMesh["end_pt_index"].GetInt());
			flat_mesh_region.add_start_index(curMesh["start_index"].GetInt());
			flat_mesh_region.add_end_index(curMesh["end_index"].GetInt());
			flat_mesh_region.add_id(curMesh["id"].GetInt());
			flat_mesh_region.add_weights(write_mesh_region_weights_list);

			mesh_region_list.push_back(flat_mesh_region.Finish());
		}
	}

	auto write_read_mesh_points = fbb.CreateVector(read_mesh_points);
	auto write_read_indices = fbb.CreateVector(read_indices);
	auto write_read_uv_points = fbb.CreateVector(read_uv_points);
	auto write_mesh_region_list = fbb.CreateVector(mesh_region_list);

	CreatureFlatData::meshBuilder flat_mesh(fbb);

	flat_mesh.add_points(write_read_mesh_points);
	flat_mesh.add_indices(write_read_indices);
	flat_mesh.add_uvs(write_read_uv_points);
	flat_mesh.add_regions(write_mesh_region_list);
	auto flat_mesh_loc = flat_mesh.Finish();


	// ----------- Process Skeleton -------------------

	std::vector<flatbuffers::Offset<CreatureFlatData::skeletonBone> > skeleton_bone_list;

	for (rapidjson::Value::MemberIterator itr = skeleton_obj.MemberBegin();
	itr != skeleton_obj.MemberEnd();
		++itr)
	{
		CreatureFlatData::skeletonBoneBuilder flat_skeleton_bone(fbb);
		auto& cur_val = itr->value;

		auto write_bone_name = fbb.CreateString(itr->name.GetString());
		auto write_restParentMat = fbb.CreateVector(GetFloatArray(cur_val["restParentMat"]));
		auto write_localRestStartPt = fbb.CreateVector(GetFloatArray(cur_val["localRestStartPt"]));
		auto write_localRestEndPt = fbb.CreateVector(GetFloatArray(cur_val["localRestEndPt"]));
		auto write_children = fbb.CreateVector(GetIntArray(cur_val["children"]));

		flat_skeleton_bone.add_name(write_bone_name);
		flat_skeleton_bone.add_id(cur_val["id"].GetInt());
		flat_skeleton_bone.add_restParentMat(write_restParentMat);
		flat_skeleton_bone.add_localRestStartPt(write_localRestStartPt);
		flat_skeleton_bone.add_localRestEndPt(write_localRestEndPt);
		flat_skeleton_bone.add_children(write_children);

		skeleton_bone_list.push_back(flat_skeleton_bone.Finish());
	}

	auto write_skeleton_bone_list = fbb.CreateVector(skeleton_bone_list);
	CreatureFlatData::skeletonBuilder flat_skeleton(fbb);

	flat_skeleton.add_bones(write_skeleton_bone_list);
	auto flat_skeleton_loc = flat_skeleton.Finish();


	// ----------- Process Animations -----------------

	std::vector<flatbuffers::Offset<CreatureFlatData::animationClip> >
		animation_clip_list;

	for (rapidjson::Value::MemberIterator itr = animation_obj.MemberBegin();
	itr != animation_obj.MemberEnd();
		++itr)
	{
		auto anim_name = itr->name.GetString();
		auto& anim_obj_val = itr->value;
		auto& anim_bone_val = anim_obj_val["bones"];
		auto& anim_mesh_val = anim_obj_val["meshes"];
		auto& anim_uv_swap_val = anim_obj_val["uv_swaps"];
		auto& anim_mesh_opacity_val = anim_obj_val["mesh_opacities"];

		// Animation Bones
		std::vector<flatbuffers::Offset<CreatureFlatData::animationBonesTimeSample> >
			animation_bone_time_sample_list;

		for (rapidjson::Value::MemberIterator c_itr = anim_bone_val.MemberBegin();
		c_itr != anim_bone_val.MemberEnd();
			++c_itr)
		{
			int cur_time = atoi(c_itr->name.GetString());

			std::vector<flatbuffers::Offset<CreatureFlatData::animationBone> > animation_bone_list;

			auto& sub_objs = c_itr->value;
			for (rapidjson::Value::MemberIterator s_itr = sub_objs.MemberBegin();
			s_itr != sub_objs.MemberEnd();
				++s_itr)
			{
				auto bone_name = s_itr->name.GetString();
				auto& cur_obj = s_itr->value;

				CreatureFlatData::animationBoneBuilder flat_animation_bone(fbb);

				auto write_bone_name = fbb.CreateString(bone_name);
				auto write_bone_start_pt = fbb.CreateVector(GetFloatArray(cur_obj["start_pt"]));
				auto write_bone_end_pt = fbb.CreateVector(GetFloatArray(cur_obj["end_pt"]));

				flat_animation_bone.add_name(write_bone_name);
				flat_animation_bone.add_start_pt(write_bone_start_pt);
				flat_animation_bone.add_end_pt(write_bone_end_pt);

				animation_bone_list.push_back(flat_animation_bone.Finish());
			}

			auto write_animation_bone_list = fbb.CreateVector(animation_bone_list);

			CreatureFlatData::animationBonesTimeSampleBuilder flat_animation_bone_time_sample(fbb);
			flat_animation_bone_time_sample.add_time(cur_time);
			flat_animation_bone_time_sample.add_bones(write_animation_bone_list);

			animation_bone_time_sample_list.push_back(flat_animation_bone_time_sample.Finish());
		}

		auto write_animation_bone_sample_list = fbb.CreateVector(animation_bone_time_sample_list);
		CreatureFlatData::animationBonesListBuilder flat_animation_bone_list(fbb);
		flat_animation_bone_list.add_timeSamples(write_animation_bone_sample_list);
		auto flat_animation_bone_list_loc = flat_animation_bone_list.Finish();

		// Animation Meshes
		std::vector<flatbuffers::Offset<CreatureFlatData::animationMeshTimeSample> >
			animation_mesh_time_sample_list;

		for (rapidjson::Value::MemberIterator c_itr = anim_mesh_val.MemberBegin();
		c_itr != anim_mesh_val.MemberEnd();
			++c_itr)
		{
			int cur_time = atoi(c_itr->name.GetString());

			std::vector<flatbuffers::Offset<CreatureFlatData::animationMesh> > animation_mesh_list;

			auto& sub_objs = c_itr->value;
			for (rapidjson::Value::MemberIterator s_itr = sub_objs.MemberBegin();
			s_itr != sub_objs.MemberEnd();
				++s_itr)
			{
				auto mesh_name = s_itr->name.GetString();
				auto& cur_obj = s_itr->value;

				auto write_mesh_name = fbb.CreateString(mesh_name);
				flatbuffers::Offset<flatbuffers::Vector<float>> write_local_displacements, write_post_displacements;

				if (cur_obj.HasMember("local_displacements"))
				{
					write_local_displacements = fbb.CreateVector(GetFloatArray(cur_obj["local_displacements"]));
				}

				if (cur_obj.HasMember("post_displacements"))
				{
					write_post_displacements = fbb.CreateVector(GetFloatArray(cur_obj["post_displacements"]));
				}

				CreatureFlatData::animationMeshBuilder flat_animation_mesh(fbb);
				flat_animation_mesh.add_name(write_mesh_name);
				flat_animation_mesh.add_use_dq(cur_obj["use_dq"].GetBool());
				flat_animation_mesh.add_use_local_displacements(cur_obj["use_local_displacements"].GetBool());
				flat_animation_mesh.add_use_post_displacements(cur_obj["use_post_displacements"].GetBool());

				if (cur_obj.HasMember("local_displacements"))
				{
					flat_animation_mesh.add_local_displacements(write_local_displacements);
				}

				if (cur_obj.HasMember("post_displacements"))
				{
					flat_animation_mesh.add_post_displacements(write_post_displacements);
				}

				animation_mesh_list.push_back(flat_animation_mesh.Finish());
			}

			auto write_animation_mesh_list = fbb.CreateVector(animation_mesh_list);

			CreatureFlatData::animationMeshTimeSampleBuilder flat_animation_mesh_time_sample(fbb);
			flat_animation_mesh_time_sample.add_time(cur_time);
			flat_animation_mesh_time_sample.add_meshes(write_animation_mesh_list);

			animation_mesh_time_sample_list.push_back(flat_animation_mesh_time_sample.Finish());
		}

		auto write_animation_mesh_time_sample_list = fbb.CreateVector(animation_mesh_time_sample_list);
		CreatureFlatData::animationMeshListBuilder flat_animation_mesh_list(fbb);
		flat_animation_mesh_list.add_timeSamples(write_animation_mesh_time_sample_list);
		auto flat_animation_mesh_list_loc = flat_animation_mesh_list.Finish();

		/// Animation UV Swaps
		std::vector<flatbuffers::Offset<CreatureFlatData::animationUVSwapTimeSample> >
			animation_uv_swap_time_sample_list;

		for (rapidjson::Value::MemberIterator c_itr = anim_uv_swap_val.MemberBegin();
		c_itr != anim_uv_swap_val.MemberEnd();
			++c_itr)
		{
			int cur_time = atoi(c_itr->name.GetString());

			std::vector<flatbuffers::Offset<CreatureFlatData::animationUVSwap> > animation_uv_swap_list;

			auto& sub_objs = c_itr->value;
			for (rapidjson::Value::MemberIterator s_itr = sub_objs.MemberBegin();
			s_itr != sub_objs.MemberEnd();
				++s_itr)
			{
				auto uv_swap_name = s_itr->name.GetString();
				auto& cur_obj = s_itr->value;

				auto write_uv_swap_name = fbb.CreateString(uv_swap_name);
				auto write_local_offset = fbb.CreateVector(GetFloatArray(cur_obj["local_offset"]));
				auto write_global_offset = fbb.CreateVector(GetFloatArray(cur_obj["global_offset"]));
				auto write_scale = fbb.CreateVector(GetFloatArray(cur_obj["scale"]));

				CreatureFlatData::animationUVSwapBuilder flat_animation_uv_swap(fbb);
				flat_animation_uv_swap.add_name(write_uv_swap_name);
				flat_animation_uv_swap.add_local_offset(write_local_offset);
				flat_animation_uv_swap.add_global_offset(write_global_offset);
				flat_animation_uv_swap.add_scale(write_scale);
				flat_animation_uv_swap.add_enabled(cur_obj["enabled"].GetBool());

				animation_uv_swap_list.push_back(flat_animation_uv_swap.Finish());
			}

			auto write_animation_uv_swap_list = fbb.CreateVector(animation_uv_swap_list);
			CreatureFlatData::animationUVSwapTimeSampleBuilder flat_animation_uv_swap_time_sample(fbb);
			flat_animation_uv_swap_time_sample.add_time(cur_time);
			flat_animation_uv_swap_time_sample.add_uvSwaps(write_animation_uv_swap_list);

			animation_uv_swap_time_sample_list.push_back(flat_animation_uv_swap_time_sample.Finish());
		}

		auto write_animation_uv_swap_time_sample_list = fbb.CreateVector(animation_uv_swap_time_sample_list);
		CreatureFlatData::animationUVSwapListBuilder flat_animation_uv_swap_list(fbb);
		flat_animation_uv_swap_list.add_timeSamples(write_animation_uv_swap_time_sample_list);
		auto flat_animation_uv_swap_list_loc = flat_animation_uv_swap_list.Finish();

		// Animation Mesh Opacities
		std::vector<flatbuffers::Offset<CreatureFlatData::animationMeshOpacityTimeSample> >
			animation_mesh_opacity_time_sample_list;

		for (rapidjson::Value::MemberIterator c_itr = anim_mesh_opacity_val.MemberBegin();
		c_itr != anim_mesh_opacity_val.MemberEnd();
			++c_itr)
		{
			int cur_time = atoi(c_itr->name.GetString());

			std::vector<flatbuffers::Offset<CreatureFlatData::animationMeshOpacity> > animation_mesh_opacity_list;

			auto& sub_objs = c_itr->value;
			for (rapidjson::Value::MemberIterator s_itr = sub_objs.MemberBegin();
			s_itr != sub_objs.MemberEnd();
				++s_itr)
			{
				auto mesh_opacity_name = s_itr->name.GetString();
				auto& cur_obj = s_itr->value;

				auto write_mesh_opacity_name = fbb.CreateString(mesh_opacity_name);

				CreatureFlatData::animationMeshOpacityBuilder flat_animation_mesh_opacity(fbb);
				flat_animation_mesh_opacity.add_name(write_mesh_opacity_name);
				flat_animation_mesh_opacity.add_opacity((float)cur_obj["opacity"].GetDouble());

				animation_mesh_opacity_list.push_back(flat_animation_mesh_opacity.Finish());
			}

			auto write_animation_mesh_opacity_list = fbb.CreateVector(animation_mesh_opacity_list);
			CreatureFlatData::animationMeshOpacityTimeSampleBuilder flat_animation_opacity_time_sample(fbb);
			flat_animation_opacity_time_sample.add_time(cur_time);
			flat_animation_opacity_time_sample.add_meshOpacities(write_animation_mesh_opacity_list);

			animation_mesh_opacity_time_sample_list.push_back(flat_animation_opacity_time_sample.Finish());
		}

		auto write_animation_mesh_opacity_time_sample_list = fbb.CreateVector(animation_mesh_opacity_time_sample_list);
		CreatureFlatData::animationMeshOpacityListBuilder flat_animation_mesh_opacity_list(fbb);
		flat_animation_mesh_opacity_list.add_timeSamples(write_animation_mesh_opacity_time_sample_list);
		auto flat_animation_mesh_opacity_list_loc = flat_animation_mesh_opacity_list.Finish();

		// Create Animation Clip
		auto write_anim_name = fbb.CreateString(anim_name);
		CreatureFlatData::animationClipBuilder flat_animation_clip(fbb);
		flat_animation_clip.add_name(write_anim_name);
		flat_animation_clip.add_bones(flat_animation_bone_list_loc);
		flat_animation_clip.add_meshes(flat_animation_mesh_list_loc);
		flat_animation_clip.add_uvSwaps(flat_animation_uv_swap_list_loc);
		flat_animation_clip.add_meshOpacities(flat_animation_mesh_opacity_list_loc);

		animation_clip_list.push_back(flat_animation_clip.Finish());
	}

	// Create Animation
	auto write_animation_clip_list = fbb.CreateVector(animation_clip_list);
	CreatureFlatData::animationBuilder flat_animation(fbb);
	flat_animation.add_clips(write_animation_clip_list);
	auto flat_animation_loc = flat_animation.Finish();

	// uv swap items
	auto& uv_swap_items_obj = read_doc["uv_swap_items"];

	std::vector<flatbuffers::Offset<CreatureFlatData::uvSwapItemMesh>> item_meshes;
	for (rapidjson::Value::MemberIterator cur_itr = uv_swap_items_obj.MemberBegin();
	cur_itr != uv_swap_items_obj.MemberEnd();
		++cur_itr)
	{
		auto mesh_name = cur_itr->name.GetString();
		auto& mesh_data = cur_itr->value;

		std::vector<flatbuffers::Offset<CreatureFlatData::uvSwapItemData>> item_list;

		for (int i = 0; i < mesh_data.Size(); i++)
		{
			CreatureFlatData::uvSwapItemDataBuilder flat_uv_swap_item_data(fbb);

			auto& m_obj = mesh_data[i];
			auto write_local_offset = fbb.CreateVector(GetFloatArray(m_obj["local_offset"]));
			auto write_global_offset = fbb.CreateVector(GetFloatArray(m_obj["global_offset"]));
			auto write_scale = fbb.CreateVector(GetFloatArray(m_obj["scale"]));

			flat_uv_swap_item_data.add_local_offset(write_local_offset);
			flat_uv_swap_item_data.add_global_offset(write_global_offset);
			flat_uv_swap_item_data.add_scale(write_scale);
			flat_uv_swap_item_data.add_tag(m_obj["tag"].GetInt());

			item_list.push_back(flat_uv_swap_item_data.Finish());
		}

		auto write_mesh_name = fbb.CreateString(mesh_name);
		auto write_item_list = fbb.CreateVector(item_list);


		CreatureFlatData::uvSwapItemMeshBuilder flat_uv_swap_item_mesh(fbb);
		flat_uv_swap_item_mesh.add_name(write_mesh_name);
		flat_uv_swap_item_mesh.add_items(write_item_list);

		item_meshes.push_back(flat_uv_swap_item_mesh.Finish());
	}

	CreatureFlatData::uvSwapItemHolderBuilder flat_uv_swap_item_holder(fbb);
	auto write_item_meshes = fbb.CreateVector(item_meshes);
	flat_uv_swap_item_holder.add_meshes(write_item_meshes);
	auto flat_uv_swap_loc = flat_uv_swap_item_holder.Finish();


	// anchor points
	auto& anchor_points_obj = read_doc["anchor_points_items"]["AnchorPoints"];

	std::vector<flatbuffers::Offset<CreatureFlatData::anchorPointData>> anchor_list;
	for (int i = 0; i < anchor_points_obj.Size(); i++)
	{
		CreatureFlatData::anchorPointDataBuilder flat_anchor_point_data_builder(fbb);

		auto& anchor_obj = anchor_points_obj[i];
		auto write_point = fbb.CreateVector(GetFloatArray(anchor_obj["point"]));
		auto write_anim_clip_name = fbb.CreateString(anchor_obj["anim_clip_name"].GetString());

		flat_anchor_point_data_builder.add_point(write_point);
		flat_anchor_point_data_builder.add_anim_clip_name(write_anim_clip_name);

		anchor_list.push_back(flat_anchor_point_data_builder.Finish());
	}

	CreatureFlatData::anchorPointsHolderBuilder flat_anchor_point_holder_builder(fbb);
	auto write_anchor_list = fbb.CreateVector(anchor_list);
	flat_anchor_point_holder_builder.add_anchorPoints(write_anchor_list);
	auto flat_anchor_loc = flat_anchor_point_holder_builder.Finish();

	// ------- Root Data -------------- //
	CreatureFlatData::rootDataBuilder flat_root(fbb);
	flat_root.add_dataSkeleton(flat_skeleton_loc);
	flat_root.add_dataMesh(flat_mesh_loc);
	flat_root.add_dataAnimation(flat_animation_loc);
	flat_root.add_dataUvSwapItem(flat_uv_swap_loc);
	flat_root.add_dataAnchorPoints(flat_anchor_loc);

	auto flat_root_loc = flat_root.Finish();

	CreatureFlatData::FinishrootDataBuffer(fbb, flat_root_loc);

	// ---- Serialize to Disk ------------- //
	remove(flat_filename_out.c_str());
	std::ofstream ofile(flat_filename_out.c_str(), std::ios::binary);
	ofile.write((char *)fbb.GetBufferPointer(), fbb.GetSize());
	ofile.close();

	std::cout << "Serialized Flat Binary File to: " << flat_filename_out << " with file size of: " << fbb.GetSize() << " bytes." << std::endl;

	return true;
}