
struct Command
{
    uint2 address;
    uint2 indexarguemnt1;
    uint3 indexarguemnt2;
    uint padding;  // important!! always remember with 64 address, there could be a 4 byte end padding at the end.
};


AppendStructuredBuffer<Command> OccCommands : register(u0);
StructuredBuffer<Command> VisCommands : register(t0);
StructuredBuffer<uint> Occresult : register(t1);

[numthreads(1024, 1, 1)]
void CSMain(uint3 groupId : SV_GroupID, uint groupIndex : SV_GroupIndex)
{
   
    uint id = groupId.x * 1024 + groupIndex;
    uint2 dim;
    VisCommands.GetDimensions(dim.x, dim.y);
    if (id < dim.x)
    {
        if (Occresult[id]==1)
        {
            OccCommands.Append(VisCommands[id]);
        }
     
    }

}