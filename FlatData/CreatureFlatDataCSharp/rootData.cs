// automatically generated, do not modify

namespace CreatureFlatData
{

using FlatBuffers;

public sealed class rootData : Table {
  public static rootData GetRootAsrootData(ByteBuffer _bb) { return GetRootAsrootData(_bb, new rootData()); }
  public static rootData GetRootAsrootData(ByteBuffer _bb, rootData obj) { return (obj.__init(_bb.GetInt(_bb.Position) + _bb.Position, _bb)); }
  public rootData __init(int _i, ByteBuffer _bb) { bb_pos = _i; bb = _bb; return this; }

  public mesh DataMesh { get { return GetDataMesh(new mesh()); } }
  public mesh GetDataMesh(mesh obj) { int o = __offset(4); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public skeleton DataSkeleton { get { return GetDataSkeleton(new skeleton()); } }
  public skeleton GetDataSkeleton(skeleton obj) { int o = __offset(6); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }
  public animation DataAnimation { get { return GetDataAnimation(new animation()); } }
  public animation GetDataAnimation(animation obj) { int o = __offset(8); return o != 0 ? obj.__init(__indirect(o + bb_pos), bb) : null; }

  public static Offset<rootData> CreaterootData(FlatBufferBuilder builder,
      Offset<mesh> dataMesh = default(Offset<mesh>),
      Offset<skeleton> dataSkeleton = default(Offset<skeleton>),
      Offset<animation> dataAnimation = default(Offset<animation>)) {
    builder.StartObject(3);
    rootData.AddDataAnimation(builder, dataAnimation);
    rootData.AddDataSkeleton(builder, dataSkeleton);
    rootData.AddDataMesh(builder, dataMesh);
    return rootData.EndrootData(builder);
  }

  public static void StartrootData(FlatBufferBuilder builder) { builder.StartObject(3); }
  public static void AddDataMesh(FlatBufferBuilder builder, Offset<mesh> dataMeshOffset) { builder.AddOffset(0, dataMeshOffset.Value, 0); }
  public static void AddDataSkeleton(FlatBufferBuilder builder, Offset<skeleton> dataSkeletonOffset) { builder.AddOffset(1, dataSkeletonOffset.Value, 0); }
  public static void AddDataAnimation(FlatBufferBuilder builder, Offset<animation> dataAnimationOffset) { builder.AddOffset(2, dataAnimationOffset.Value, 0); }
  public static Offset<rootData> EndrootData(FlatBufferBuilder builder) {
    int o = builder.EndObject();
    return new Offset<rootData>(o);
  }
  public static void FinishrootDataBuffer(FlatBufferBuilder builder, Offset<rootData> offset) { builder.Finish(offset.Value); }
};


}
