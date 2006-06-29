#include <gtk/gtk.h>


void
on_button_gallery_open_clicked         (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_gallery_save_clicked         (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_gallery_make_clicked         (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_gallery_stop_clicked         (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_gallery_configure_clicked    (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_image_add_clicked            (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_image_remove_clicked         (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_pwg_preferences_clicked      (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_pwg_quit_clicked             (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_pwg_about_clicked            (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_button_move_top_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_move_up_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_move_down_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_move_bottom_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_check_settings_nomodify_toggled     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_scale_settings_gamma_value_changed  (GtkRange        *range,
                                        gpointer         user_data);

void
on_scale_settings_rotate_value_changed (GtkRange        *range,
                                        gpointer         user_data);

void
on_button_pref_save_clicked            (GtkToolButton   *toolbutton,
                                        gpointer         user_data);
