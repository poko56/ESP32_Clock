// =============================================================
//  ESP32 Desk Clock enclosure
//  ESP32 DevKit (30-pin) + OLED 0.96" + DS3231 + FC-51 IR + LDR + DHT11
//
//  Coordinates: x = left->right (seen from the front), y = front->back,
//  z = up.  Origin = inner front-bottom-left corner.  The front wall is at
//  y = -front_t..0, the back cover closes y = D.
//
//  part = "body" | "body_print" | "cover" | "cover_print"
//       | "assembly" | "exploded" | "section"
//
//  Print: body front face down (no supports), cover outside face down.
//  Screws: 4 x M3x10 self-tapping (or M3 heat-set inserts: set m3_pilot = 4.0)
// =============================================================
part = "assembly";

$fn = 48;
e = 0.01;

// ---------- box ----------
W = 100;          // inner width
D = 52;           // inner depth (front wall -> back cover)
H = 54;           // inner height
wall = 2.4;
front_t = 2.0;
cover_t = 2.5;
r_out = 5;        // outer corner radius (edges along y)
chamfer = 0.8;    // front + back outer edge chamfer
OW = W + 2 * wall;
OH = H + 2 * wall;

// ---------- screw columns (inner corners) ----------
col_r = 3.6;
m3_pilot = 2.6;
m3_clear = 3.4;
m3_head_d = 6.6;
col_c = [[col_r, col_r], [W - col_r, col_r], [col_r, H - col_r], [W - col_r, H - col_r]];

// ---------- OLED 0.96" (4-pin, pins on top) — measured 24.8 x 27.1 ----------
oled_w = 24.8;  oled_h = 27.1;     // PCB (along the pins x across)
oled_x = 16;    oled_z = 20;       // PCB lower-left corner (seen from front)
glass_t = 1.4;                     // glass sits flat on the inside of the front wall
glass_vis_top = 5.5;               // pin edge -> top of the dark glass (from photo)
glass_vis_h = 17;                  // visible dark glass height (from photo)
win_from_top = 13.2;               // pin edge -> centre of the pixel area
win_w = 23.5;   win_h = 13.5;      // viewing window (pixels are ~21.7 x 10.9)
glass_cx = oled_x + oled_w / 2;
glass_cz = oled_z + oled_h - win_from_top;
oled_frame_t = 1.2;                // locating frame around the PCB
oled_gap = 0.3;

// ---------- IR obstacle sensor FC-51 / MH-B — measured 14.3 wide, 42.3 incl. LEDs ----------
ir_w = 14.3;  ir_len = 30.8;       // PCB
ir_cx = 76;                        // centre x
ir_led_up = 2.6;                   // LED centre above PCB bottom (LEDs stick straight out)
ir_pcb_z = glass_cz - ir_led_up;   // LEDs line up with the display centre
ir_led_reach = 11.5;               // LED tip past PCB front edge
ir_y0 = ir_led_reach - front_t;    // PCB front edge -> LED tips flush with face
ir_led_dx = 3.8;                   // each LED from centre (7.6 mm apart)
ir_slot_w = 15.5; ir_slot_h = 6.6;

// ---------- ESP32 DevKit 30-pin (USB to the left wall) ----------
esp_len = 51.5; esp_w = 28.5; esp_t = 1.6;
esp_x0 = 1.2;                      // USB end of the PCB
esp_y0 = 16;
esp_z = 14;                        // PCB bottom (header pins hang below)
esp_grip = 2.2;                    // antenna end slides into a slot this deep
esp_cy = esp_y0 + esp_w / 2;
usb_w = 12; usb_h = 8;
usb_cz = esp_z + esp_t + 1.5;

// ---------- DS3231 (lies on the floor) ----------
ds_w = 22; ds_len = 38;
ds_x0 = 66; ds_y0 = 7;

// ---------- DHT11 module (sensor pokes out of the right wall) — PCB 16.5 x 21.8 ----------
dht_body_w = 15.5; dht_body_h = 12;  // blue sensor body (y, z), flush with the PCB front end + lower edge
dht_body_t = 5.5;
dht_pcb_h = 16.5;  dht_pcb_len = 21.8;
dht_y0 = 16;                       // PCB front end
dht_pcb_z0 = 20;                   // PCB lower edge
dht_cz = dht_pcb_z0 + dht_body_h / 2;   // centre of the sensor body
dht_pcb_cz = dht_pcb_z0 + dht_pcb_h / 2;

// ---------- LDR (head through the top) ----------
ldr_x = 86; ldr_y = 10; ldr_d = 5.6;

// =============================================================
//  helpers
// =============================================================
// extrude a 2D (x, z) shape along +y from y0, length len
module along_y(y0, len) translate([0, y0 + len, 0]) rotate([90, 0, 0]) linear_extrude(len) children();

module rrect(w, h, r) translate([r, r]) offset(r) square([w - 2 * r, h - 2 * r]);
module rrect_c(w, h, r) translate([-w / 2, -h / 2]) rrect(w, h, r);

module outer2d(inset = 0) translate([-wall + inset, -wall + inset])
  rrect(OW - 2 * inset, OH - 2 * inset, max(0.6, r_out - inset));

module columns2d() for (c = col_c) hull() {
  translate(c) circle(col_r);
  translate([c[0] < W / 2 ? 0 : W - col_r, c[1] < H / 2 ? 0 : H - col_r]) square(col_r);
}

// rounded box along x (for holes in the side walls)
module rbox_x(x0, len, yc, zc, wy, hz, r) hull()
  for (yy = [yc - wy / 2 + r, yc + wy / 2 - r], zz = [zc - hz / 2 + r, zc + hz / 2 - r])
    translate([x0, yy, zz]) rotate([0, 90, 0]) cylinder(r = r, h = len);

// through-hole in the front wall with an outer chamfer
module front_hole(cx, cz, w, h, r) {
  along_y(-front_t - 1, front_t + 2) translate([cx, cz]) rrect_c(w, h, r);
  hull() {
    along_y(-front_t - e, e) translate([cx, cz]) rrect_c(w + 1.6, h + 1.6, r + 0.8);
    along_y(-front_t + 0.8, e) translate([cx, cz]) rrect_c(w, h, r);
  }
}

// =============================================================
//  body
// =============================================================
module shell() difference() {
  hull() {
    along_y(-front_t, e) outer2d(chamfer);
    along_y(-front_t + chamfer, D + front_t - chamfer) outer2d(0);
  }
  along_y(0, D + 1) square([W, H]);
}

// frame around the OLED PCB: glass flat on the wall, PCB edges located by the frame
module oled_frame() along_y(0, glass_t + 1.2 + 1.0) difference() {
  translate([oled_x - oled_gap - oled_frame_t, oled_z - oled_gap - oled_frame_t])
    square([oled_w + 2 * (oled_gap + oled_frame_t), oled_h + 2 * (oled_gap + oled_frame_t)]);
  translate([oled_x - oled_gap, oled_z - oled_gap]) square([oled_w + 2 * oled_gap, oled_h + 2 * oled_gap]);
  // opening at the top for the 4 wires
  translate([oled_x + 4, oled_z + oled_h - 1]) square([oled_w - 8, 5]);
}

module esp_cradles() {
  // antenna end: block with a slot, the board slides in from the back
  xe = esp_x0 + esp_len;
  difference() {
    // runs from the front wall so it prints without an overhang
    translate([xe - esp_grip, 0, 0])
      cube([esp_grip + 3.5, D - 2.5, esp_z + esp_t + 0.4 + 2.2]);
    translate([xe - esp_grip - e, esp_y0, esp_z]) cube([esp_grip + 0.4 + e, D, esp_t + 0.4]);
  }
  // USB end: ledge between the two header rows
  translate([-e, esp_cy - 8, 0]) cube([esp_x0 + 2.0, 16, esp_z]);
}

module ir_rails() {
  gh = ir_w / 2 + 0.3;    // half width of the channel
  for (s = [-1, 1]) {
    // outer wall
    translate([ir_cx + (s < 0 ? -gh - 2.5 : gh), 0, ir_pcb_z - 4]) cube([2.5, ir_y0 + ir_len - 2, 8]);
    // ledge the PCB rests on (starts behind the LEDs)
    translate([ir_cx + (s < 0 ? -gh : gh - 1.5), ir_y0, ir_pcb_z - 4]) cube([1.5, ir_len - 2, 4]);
    // stop so the PCB can't slide forward
    translate([ir_cx + (s < 0 ? -gh : gh - 1.5), ir_y0 - 1.5, ir_pcb_z - 4]) cube([1.5, 1.5, 4 + 1.6]);
  }
}

module ds_rails()
  for (x = [ds_x0 - 1.7, ds_x0 + ds_w + 0.5]) translate([x, ds_y0, 0]) cube([1.2, ds_len, 3]);

module dht_guides()
  for (z = [dht_pcb_z0 - 0.3 - 2, dht_pcb_z0 + dht_pcb_h + 0.3])
    translate([W - 4, dht_y0 - 2, z]) cube([4 + e, dht_pcb_len + 2, 2]);

module body() difference() {
  union() {
    shell();
    along_y(0, D) columns2d();
    oled_frame();
    esp_cradles();
    ir_rails();
    ds_rails();
    dht_guides();
  }
  // OLED viewing window
  front_hole(glass_cx, glass_cz, win_w, win_h, 1.2);
  // IR window
  front_hole(ir_cx, ir_pcb_z + ir_led_up, ir_slot_w, ir_slot_h, ir_slot_h / 2 - 0.01);
  // LDR
  translate([ldr_x, ldr_y, H - 1]) cylinder(d = ldr_d, h = wall + 2);
  // DHT11 window
  rbox_x(W - 1, wall + 2, dht_y0 + dht_body_w / 2, dht_cz, dht_body_w + 0.8, dht_body_h + 0.8, 1);
  // micro-USB
  rbox_x(-wall - 1, wall + 2, esp_cy, usb_cz, usb_w, usb_h, 1.5);
  // bottom vents under the ESP32
  for (x = [10 : 5 : 45]) translate([x, esp_y0 + 3, -wall - 1]) cube([2, esp_w - 6, wall + 2]);
  // screw pilots
  for (c = col_c) translate([c[0], D - 14, c[1]]) rotate([-90, 0, 0]) cylinder(d = m3_pilot, h = 14 + e);
}

// =============================================================
//  back cover
// =============================================================
lip_d = 2; lip_w = 1.6; lip_gap = 0.3;

module cover() difference() {
  union() {
    hull() {
      along_y(D, cover_t - chamfer) outer2d(0);
      along_y(D + cover_t - e, e) outer2d(chamfer);
    }
    // locating lip that sits just inside the walls
    along_y(D - lip_d, lip_d + e) difference() {
      translate([lip_gap, lip_gap]) square([W - 2 * lip_gap, H - 2 * lip_gap]);
      translate([lip_gap + lip_w, lip_gap + lip_w]) square([W - 2 * (lip_gap + lip_w), H - 2 * (lip_gap + lip_w)]);
      offset(0.4) columns2d();
    }
  }
  // countersunk M3
  for (c = col_c) translate([c[0], 0, c[1]]) {
    translate([0, D - 1, 0]) rotate([-90, 0, 0]) cylinder(d = m3_clear, h = cover_t + 2);
    translate([0, D + cover_t - 1.7, 0]) rotate([-90, 0, 0]) cylinder(d1 = m3_clear, d2 = m3_head_d, h = 1.7 + e);
  }
  // vents behind the ESP32 and high up for warm air
  for (x = [9 : 5 : 49]) translate([x, D - 1, 22]) cube([2, cover_t + 2, 18]);
  // engraved name (reads correctly from the back)
  along_y(D + cover_t - 0.6, 1) translate([W / 2, 6]) mirror([1, 0])
    text("ESP32 CLOCK", size = 5, halign = "center", font = "Liberation Sans:style=Bold");
}

// =============================================================
//  stand-in modules (for checking the fit only)
// =============================================================
module phantoms() {
  // OLED
  color("royalblue") translate([oled_x, glass_t, oled_z]) cube([oled_w, 1.2, oled_h]);
  color("black") translate([oled_x + 0.3, 0, oled_z + oled_h - glass_vis_top - glass_vis_h]) cube([oled_w - 0.6, glass_t, glass_vis_h]);
  color("gold") translate([oled_x + 7.3, 2.6, oled_z + oled_h - 2.5]) cube([10.2, 6, 1]);
  // ESP32
  color("#202020") translate([esp_x0, esp_y0, esp_z]) cube([esp_len, esp_w, esp_t]);
  color("silver") translate([esp_x0 + esp_len - 25.5, esp_cy - 9, esp_z + esp_t]) cube([25.5, 18, 3.2]);
  color("silver") translate([esp_x0 - 0.6, esp_cy - 3.9, esp_z + esp_t]) cube([5.6, 7.8, 2.8]);
  color("gold") for (y = [esp_y0 + 0.75, esp_y0 + esp_w - 3.25])
    translate([esp_x0 + 4, y, esp_z - 11.5]) cube([38.1, 2.5, 11.5]);
  // IR sensor
  color("royalblue") translate([ir_cx - ir_w / 2, ir_y0, ir_pcb_z]) cube([ir_w, ir_len, 1.6]);
  color("white") translate([ir_cx - ir_led_dx, ir_y0, ir_pcb_z + ir_led_up]) rotate([90, 0, 0]) cylinder(d = 5, h = ir_led_reach);
  color("#111") translate([ir_cx + ir_led_dx, ir_y0, ir_pcb_z + ir_led_up]) rotate([90, 0, 0]) cylinder(d = 5, h = ir_led_reach);
  color("dodgerblue") translate([ir_cx - 3.5, ir_y0 + 12, ir_pcb_z + 1.6]) cube([7, 7, 5]);
  // DS3231
  color("royalblue") translate([ds_x0, ds_y0, 5]) cube([ds_w, ds_len, 1.6]);
  color("#222") translate([ds_x0 + ds_w / 2, ds_y0 + 14, 0]) cylinder(d = 21, h = 5);
  color("gold") for (y = [ds_y0 - 6, ds_y0 + ds_len]) translate([ds_x0 + 4, y, 6]) cube([14, 6, 1]);
  // DHT11
  color("#202020") translate([W - 1.6, dht_y0, dht_pcb_z0]) cube([1.6, dht_pcb_len, dht_pcb_h]);
  color("lightskyblue") translate([W, dht_y0, dht_pcb_z0]) cube([dht_body_t, dht_body_w, dht_body_h]);
  // LDR module
  color("royalblue") translate([ldr_x - 7, ldr_y - 4, H - 7]) cube([14, 30, 1.6]);
  color("orange") translate([ldr_x, ldr_y, H - 3]) cylinder(d = 5, h = 2 + wall);
}

// =============================================================
if (part == "body") body();
else if (part == "body_print") translate([0, 0, front_t]) rotate([90, 0, 0]) body();
else if (part == "cover") cover();
else if (part == "cover_print") translate([0, 0, D + cover_t]) rotate([-90, 0, 0]) cover();
else if (part == "assembly") { color("#e8e4dc") body(); color("#cfc9bd") cover(); phantoms(); }
else if (part == "exploded") { color("#e8e4dc") body(); translate([0, 45, 0]) color("#cfc9bd") cover(); phantoms(); }
else if (part == "section") {
  difference() { union() { color("#e8e4dc") body(); color("#cfc9bd") cover(); } translate([-10, -10, glass_cz]) cube([OW + 20, D + 20, 40]); }
  phantoms();
}
