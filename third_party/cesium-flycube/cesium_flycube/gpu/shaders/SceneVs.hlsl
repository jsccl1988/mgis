struct VsInput {
    float3 pos : POSITION;
    float3 color : COLOR;
};

struct VsOutput {
    float4 pos : SV_POSITION;
    float3 color : COLOR;
};

VsOutput main(VsInput input) {
    VsOutput output;
    // Positions are already in clip/NDC from the CPU projector.
    output.pos = float4(input.pos, 1.0);
    output.color = input.color;
    return output;
}
