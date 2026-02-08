import asyncio
import asyncpg

DB_URL = "postgresql://tess:tess@107.189.20.246:5662/tess"

# Data provided by user from GitHub Release
INSTALLERS = [
    {
        "os_type": "windows",
        "filename": "tess-v1.0.0-installer.msi",
        "url": "https://github.com/egirlst/tess/releases/download/v1.0.0/tess-v1.0.0-installer.msi",
        "shash": "10df2e14411b9ef1ea6058f33c434618bbac67375b31e760b732ba7bdd81b72e",
        "size": 332 * 1024 # 332 KB
    },
    {
        "os_type": "macos",
        "filename": "tess-installer-v1.pkg",
        "url": "https://github.com/egirlst/tess/releases/download/v1.0.0/tess-installer-v1.pkg",
        "shash": "728ae12982ba7abf0892142d132b36d8d2177ab22adfcf41fead82c1533a051e",
        "size": 21 # 21 Bytes
    },
    {
        "os_type": "linux",
        "filename": "tess-1.0.0.tar.gz",
        "url": "https://github.com/egirlst/tess/releases/download/v1.0.0/tess-1.0.0.tar.gz",
        "shash": "34e7aa6fd307f2810dfe16af67b8d2169f3cefd87dac7402695b993a0099d278",
        "size": 3717 # 3.63 KB approx 3717 bytes
    },
    {
        "os_type": "linux",
        "filename": "tess.deb",
        "url": "https://github.com/egirlst/tess/releases/download/v1.0.0/tess.deb",
        "shash": "01bc7a3e79a4babd8b1ee0f070d953216010eb354d51458d7387365155317651",
        "size": 21 # 21 Bytes
    }
]

async def insert_installer(conn, change_id, installer):
    # Check if exists by filename (path) to allow multiple files per OS
    row = await conn.fetchrow("SELECT id FROM installers WHERE change_id = $1 AND path = $2", change_id, installer["filename"])
    
    if row:
        print(f"Installer {installer['filename']} exists. Updating...")
        await conn.execute("""
            UPDATE installers 
            SET shash = $1, size_bytes = $2, url = $3, os_type = $4
            WHERE id = $5
        """, installer["shash"], installer["size"], installer["url"], installer["os_type"], row['id'])
        print(f"Updated {installer['filename']}")
    else:
        await conn.execute("""
            INSERT INTO installers (change_id, os_type, shash, url, path, size_bytes)
            VALUES ($1, $2, $3, $4, $5, $6)
        """, change_id, installer["os_type"], installer["shash"], installer["url"], installer["filename"], installer["size"])
        print(f"Inserted {installer['filename']}")

async def main():
    print(f"Connecting to {DB_URL}...")
    try:
        conn = await asyncpg.connect(DB_URL)
    except Exception as e:
        print(f"Failed to connect to DB: {e}")
        return

    try:
        # 1. Ensure Tables Exist
        print("Ensuring tables exist...")
        await conn.execute("""
            CREATE TABLE IF NOT EXISTS releases (
                id SERIAL PRIMARY KEY,
                name VARCHAR(255) NOT NULL,
                version_number VARCHAR(50) NOT NULL,
                version_group VARCHAR(50),
                pushed_by VARCHAR(100),
                push_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                release_type VARCHAR(20) NOT NULL,
                status VARCHAR(20) CHECK (status IN ('draft', 'stable', 'deprecated')),
                note TEXT,
                UNIQUE (name, version_number)
            );
            
            CREATE TABLE IF NOT EXISTS sub_versions (
                id SERIAL PRIMARY KEY,
                release_id INT NOT NULL REFERENCES releases(id) ON DELETE CASCADE,
                version VARCHAR(50) NOT NULL,
                UNIQUE (release_id, version)
            );
            
            CREATE TABLE IF NOT EXISTS sub_changes (
                id SERIAL PRIMARY KEY,
                sub_version_id INT NOT NULL REFERENCES sub_versions(id) ON DELETE CASCADE,
                change_version VARCHAR(50) NOT NULL,
                note TEXT
            );
            
            CREATE TABLE IF NOT EXISTS installers (
                id SERIAL PRIMARY KEY,
                change_id INT NOT NULL REFERENCES sub_changes(id) ON DELETE CASCADE,
                os_type VARCHAR(20) NOT NULL CHECK (os_type IN ('linux', 'macos', 'windows')),
                shash VARCHAR(255) NOT NULL,
                url TEXT NOT NULL,
                path TEXT NOT NULL,
                size_bytes BIGINT,
                checksum_algo VARCHAR(20) DEFAULT 'sha256'
            );
        """)

        # 1.5 Try to update constraints
        try:
            await conn.execute("ALTER TABLE releases DROP CONSTRAINT IF EXISTS releases_release_type_check")
            await conn.execute("ALTER TABLE releases ADD CONSTRAINT releases_release_type_check CHECK (release_type IN ('Major', 'Minor', 'Bugfix', 'Security', 'End-of-life'))")
        except Exception as e:
            pass

        # 2. Insert Release 1.0.0
        print("Checking for Release 1.0.0...")
        release_id = await conn.fetchval("SELECT id FROM releases WHERE name = $1 AND version_number = $2", "Tess", "1.0.0")
        
        if not release_id:
            print("Creating Release 1.0.0...")
            release_id = await conn.fetchval("""
                INSERT INTO releases (name, version_number, version_group, pushed_by, release_type, status, note)
                VALUES ($1, $2, $3, $4, $5, $6, $7)
                RETURNING id
            """, "Tess", "1.0.0", "1.x", "sa1nt", "Major", "stable", "Initial Release")
            print(f"Created Release 1.0.0 with ID {release_id}")
        else:
            print(f"Release 1.0.0 already exists with ID {release_id}")

        # 3. Insert SubVersion
        sub_ver_id = await conn.fetchval("SELECT id FROM sub_versions WHERE release_id = $1 AND version = $2", release_id, "1.0.0")
        if not sub_ver_id:
            sub_ver_id = await conn.fetchval("""
                INSERT INTO sub_versions (release_id, version)
                VALUES ($1, $2)
                RETURNING id
            """, release_id, "1.0.0")
            print(f"Created SubVersion 1.0.0 with ID {sub_ver_id}")

        # 4. Insert SubChange
        sub_change_id = await conn.fetchval("SELECT id FROM sub_changes WHERE sub_version_id = $1 AND change_version = $2", sub_ver_id, "1.0.0")
        if not sub_change_id:
            sub_change_id = await conn.fetchval("""
                INSERT INTO sub_changes (sub_version_id, change_version, note)
                VALUES ($1, $2, $3)
                RETURNING id
            """, sub_ver_id, "1.0.0", "Initial release artifacts")
            print(f"Created SubChange 1.0.0 with ID {sub_change_id}")

        # 5. Insert Installers
        for installer in INSTALLERS:
            await insert_installer(conn, sub_change_id, installer)

        print("Database population complete.")

    finally:
        await conn.close()

if __name__ == "__main__":
    asyncio.run(main())
