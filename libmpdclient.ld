libmpdclient2 {
global:
	/* mpd/async.h */
	mpd_async_new;
	mpd_async_free;
	mpd_async_is_alive;
	mpd_async_get_error;
	mpd_async_get_error_message;
	mpd_async_fd;
	mpd_async_events;
	mpd_async_io;
	mpd_async_send_command_v;
	mpd_async_send_command;
	mpd_async_recv_line;

	/* mpd/send.h */
	mpd_send_command;

	/* mpd/command.h */
	mpd_send_currentsong;
	mpd_send_play;
	mpd_send_playid;
	mpd_send_stop;
	mpd_send_pause;
	mpd_send_next;
	mpd_send_previous;
	mpd_send_seek;
	mpd_send_seekid;
	mpd_send_repeat;
	mpd_send_random;
	mpd_send_single;
	mpd_send_consume;
	mpd_send_crossfade;
	mpd_send_playlistinfo;
	mpd_send_playlistid;
	mpd_send_plchanges;
	mpd_send_plchangesposid;
	mpd_send_add;
	mpd_send_addid;
	mpd_send_delete;
	mpd_send_deleteid;
	mpd_send_shuffle;
	mpd_send_shuffle_range;
	mpd_send_clear;
	mpd_send_move;
	mpd_send_moveid;
	mpd_send_swap;
	mpd_send_swapid;
	mpd_send_listplaylist;
	mpd_send_listplaylistinfo;
	mpd_send_playlistclear;
	mpd_send_playlistadd;
	mpd_send_playlistmove;
	mpd_send_playlistdelete;
	mpd_send_save;
	mpd_send_load;
	mpd_send_rename;
	mpd_send_rm;
	mpd_send_listall;
	mpd_send_listallinfo;
	mpd_send_lsinfo;
	mpd_send_list_artist;
	mpd_send_list_album;
	mpd_send_update;
	mpd_send_setvol;
	mpd_send_volume;
	mpd_send_password;
	mpd_send_commands;
	mpd_send_notcommands;
	mpd_send_urlhandlers;
	mpd_send_tagtypes;

        *;
};
