/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

layout(location = 0) in vec3 texcoord;

layout(location = 0) out vec4 color_0;

layout(binding = 0) uniform sampler2DArray sampler_0;

void main()
{
    color_0 = texture(sampler_0, texcoord);
}
