//
// This is a rough example of the current capabilities of `qsdl`
//

\l sdl2.q

// SDL Init
screen_width: 1280i;
screen_height: 720i;
sdl_init ("example";screen_width;screen_height);

// Set up loop (and fps)
.z.ts: { sdl_tick[]; run_loop[]; }
run_fps: 30;
system "t ",string `int$ 1000 % run_fps;

sdl_show_simple_message_box[`info;"Example";"This is an example of a qsdl2 program.\nIt will show events on the q console.\nTry moving the mouse and clicking in the window, pressing keys, etc.";1];

// Wrap `sdl_poll_event` to convert events into a single table
sdl_poll_event_fn: sdl_poll_event;
sdl_poll_event: {
  e: sdl_poll_event_fn[];
  et: flip (`etype`typecode`timestamp`data)!(flip { (x`etype;x`typecode;x`timestamp;x`data) } each e);
  et
  };

// Flatten event specific fields within data to table columns
eventflat: {[typ;t]
  et: select from t where etype in typ;
  (delete data from et)^(exec data from et)
  };

// State tracking
key_state: ([keyname: ()] state: (); timestamp: ());
mb_state: ([button: ()] state: (); timestamp: (); x: (); y: ());
mm_state: ([which: ()] state: (); timestamp: (); x: (); y: (); xrel: (); yrel: ());
debug: 1b;

bgc: 0x000000;

n: 1000000;
genpoints: {[n] `int$ raze flip (n ? screen_width ; n ? screen_height) };
genrects: {[n] `int$ raze flip (n ? screen_width ; n ? screen_height ; n # 20 ; n # 20)};
rects: genrects 100;
gpoints: genpoints 1000000;

clamp: { x: ?[x > 255; 255; x]; x: ?[x < 0; 0; x]; `byte$x };

run_loop:  {
  evt: sdl_poll_event[];
  
  if[debug; all_evt,:: evt];

  kevt:  eventflat[`keydown`keyup] evt;
  mbevt: eventflat[`mousebuttonup`mousebuttondown] evt;
  mmevt: eventflat[`mousemotion] evt;
  mwevt: eventflat[`mousewheel] evt;

  if[(count kevt) > 0;
    `key_state upsert select last state, last timestamp by keyname from kevt;
    show kevt;
    show key_state;
  ];
  if[(count mbevt) > 0;
    `mb_state upsert select last state, last timestamp, last x, last y by button from mbevt;
    show mbevt;
    show mb_state;
  ];
  if[(count mmevt) > 0;
    `mm_state upsert select last state, last timestamp, last x, last y, last xrel, last yrel by which from mmevt;
    show mmevt;
    show mm_state;
  ];
  if[(count mwevt) > 0;
    show mwevt;
  ];

  if[`quit in evt[`etype]; :exit 0];

  if[`mousebuttondown in evt[`etype];
    bgc:: `byte$ 3 ? 255;
  ];

  if[(count mwevt) > 0;
    adj: exec last y from mwevt;
    bgc:: clamp bgc + adj;
  ];

  sdl_render_clear[1i; bgc,0xff];

  rects:: raze flip (`int$ 20 * til 40; `int$ 40 + 5.0 * 40#sin (`float$ (`int$.z.t mod 100) + til 10); `int$ 40#30; `int$ 40#30);

  sdl_render_draw_points[1i; 0xff0000ff; gpoints];

  //sdl_render_draw_lines[1i; 0x00ff00ff; lines];
  //sdl_render_fill_rects[1i; 0xff0000aa] rects;

  sdl_render_present[1i];
  };

