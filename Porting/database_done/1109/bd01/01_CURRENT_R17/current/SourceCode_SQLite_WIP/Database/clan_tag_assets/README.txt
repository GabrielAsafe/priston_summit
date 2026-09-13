# Clan tag source assets (local server storage)

Place the selectable clan-tag BMP assets here using the legacy item-code basename:

    Database/clan_tag_assets/<CodeIT>.bmp

When a clan leader selects a tag, the server copies that BMP to:

    Database/clan_marks/<ClanMark>.bmp

This replaces the legacy IIS paths:

    C:\inetpub\wwwroot\brnxcontenttags\<CodeIT>.bmp
    C:\inetpub\wwwroot\ClanContent\<ClanMark>.bmp

The legacy code ignored the result of the shell `copy` command and continued the
purchase/update flow. The SQLite/local-storage port preserves that observable
behavior: a missing source asset does not change the existing return/message flow.

No default clan-mark bitmap is synthesized by the migration.
