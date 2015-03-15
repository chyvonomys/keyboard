inch = 25.4;
keyboard_unit = 0.75 * inch;

plate_thickness = 0.06 * inch;

keys =
[
    [ 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,    2 ],
    [ 1.5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1.5 ],
    [ 1.75,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2.25   ],
    [ 2.25,    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2.75     ],
    [ 1.25,1.25,1,1.25,2, 2,    1.25,1,1,  1,  1,  1]
];

cmd = "\u2318";
alt = "\u2325";
shift = "\u21E7";
left = "\u2190";
right = "\u2192";
up = "\u2191";
down = "\u2193";
bksp = "\u232B";
del = "\u232B";

labels = 
[
    ["`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "delete"],
    ["tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\"],
    ["esc", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "enter"],
    ["shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "shift"],
    ["fn", "cmd", "alt", "ctrl", "  ", "  ", "ctrl", "alt", left, down, up, right]
];


function key_color(r, n) =
(n == "esc" || n == "tab" || n == "delete" || n == "enter" || n == "shift" ||
    n == "alt" || n == "ctrl" || n == "fn" || n == "cmd" ? "skyblue" :
    "white");

function sumv(v, i) = (i <= 0 ? 0 : abs(v[i-1]) + sumv(v, i-1));

rows = len(keys);
columns = max([for (r = [0:rows-1]) sumv(keys[r], len(keys[r]))]);
    
echo(rows = rows, columns = columns);

plate_width = columns * keyboard_unit;
plate_height = rows * keyboard_unit;

plate_size = [plate_width, plate_height, plate_thickness];

module switch(stabilized)
{
    stab_offset_y = 0.03 * inch;
    stab_offset_x = 0.47 * inch;
    stab_hole_width = 0.13 * inch;
    stab_hole_height = 0.551 * inch;
    switch_hole_width = 0.551 * inch;
    switch_hole_height = 0.551 * inch;

    switch_hole = [switch_hole_width, switch_hole_height, 10];
    stab_hole = [stab_hole_width, stab_hole_height, 10];

    cube(size = switch_hole, center = true);
    if (stabilized)
    {
	translate([-stab_offset_x, -stab_offset_y, 0])
	cube(size = stab_hole, center = true);

	translate([+stab_offset_x, -stab_offset_y, 0])
	cube(size = stab_hole, center = true);
    }
}

// plate
padding = 7;
//projection()
difference() {
    color("black")
    cube(size = plate_size + [padding, padding, 0], center = true);
    color("white")
    translate([-plate_size[0]/2, -plate_size[1]/2, 0])
    {
	for (r = [0:4])
	{
	    for (c = [0:len(keys[r])-1])
	    {
                w = keys[r][c];
		if (w > 0)
		{
		    translate(keyboard_unit * [(sumv(keys[r], c) + w * 0.5), (rows - r - 0.5), 0])
		    switch(stabilized = (w >= 2));
		}
	    }
	}
    }
}

module label(text)
{
    text(text = text, font="Lucida Grande", size = keyboard_unit/4, halign="center", valign="center");
}

// keycaps
translate([-plate_size[0]/2, 20+plate_size[1]/2, 5])
{
    for (r = [0:4])
    {
	for (c = [0:len(keys[r])-1])
	{
            w = keys[r][c];
	    if (w > 0)
	    {
		translate(keyboard_unit * [(sumv(keys[r], c) + w * 0.5), (rows - r - 0.5), 0])
                {
                    color(key_color(r, labels[r][c]))
                    linear_extrude(height=keyboard_unit/2, scale=[(w-0.2)/w,(1-0.2)/1])
                    square(size=keyboard_unit * [w-0.05, 1-0.05], center=true);
                    
                    color("black")
                    translate([0, 0, keyboard_unit/2+0.1])
		    scale([1,1,0.1])
		    label(labels[r][c]);
                }
	    }
	}
    }
}

cap_width = 0.715 * inch;
cap_height = 0.715 * inch;

face_width = 0.47 * inch;
face_height = 0.557 * inch;
