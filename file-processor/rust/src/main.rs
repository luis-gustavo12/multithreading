use std::collections::HashMap;
use std::fmt::{Display, Formatter};
use std::fs::DirEntry;
use std::path::PathBuf;
use std::sync::{Arc, Mutex};
use std::thread::JoinHandle;
use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {

    #[arg(short, long, value_parser = validate_path)]
    path: PathBuf,

}

fn validate_path(path: &str) -> Result<std::path::PathBuf, String> {
    let p = std::path::PathBuf::from(path);
    if !p.exists() {
        return Err(format!("Path does not exist: {}", path));
    }
    if p.is_file() {
        return Err(format!("Expected a directory, got a file: {}", path));
    }
    Ok(p)
}



#[derive(Debug, Clone)]
enum EntityType {
    File,
    Directory,
}

#[derive(Debug, Clone)]
struct Entity {
    entity_type: EntityType,
    size: u64,
    path: PathBuf,
    file_ext: Option<String>,
    name: String,
    created_at: Option<chrono::DateTime<chrono::Utc>>,
    thread_number: u32
}

fn get_extension(file_name: &str) -> Option<String> {
    let parts: Vec<&str> = file_name.split('.').collect();

    if parts.len() < 2 {
        return None;
    }

    Some(parts.last().unwrap().to_string())
}

impl Entity {

    pub fn from_dir_entry(dir_entry: &DirEntry, thread_number: u32) -> Option<Self> {

        let path = dir_entry.path();
        let file_ext = get_extension(path.to_str().unwrap());

        if file_ext.is_none() {
            return None;
        }

        let metadata = dir_entry.metadata().ok().unwrap();

        let created_at_opt = metadata.created().ok().map(
            chrono::DateTime::from
        );


        Some(
            Entity {
                entity_type: EntityType::File,
                size: metadata.len(),
                path: path.clone(),
                file_ext: get_extension(path.to_str().unwrap()),
                name: path.to_str().unwrap().to_string(),
                created_at: created_at_opt,
                thread_number,
            }
        )



    }

}

struct Report {
    total_size: u64,
    total_files: u64,
    processed_entities: Vec<Entity>,
}

impl Report {
    pub fn new(total_size: u64, total_files: u64, processed_entities: Vec<Entity> ) -> Self {
        Self {
            total_size,
            total_files,
            processed_entities,
        }
    }
}

impl Display for Report {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {

        let mut threads: Vec<u32> = self.processed_entities.iter().map(|e| e.thread_number).collect();
        threads.sort_unstable();
        threads.dedup();

        let no_threads = threads.len();
        let mut display_entities: HashMap<u32, Vec<&Entity>> = HashMap::with_capacity(no_threads);

        // Initialize the hash map
        for i in 0..no_threads {
            display_entities.insert(i as u32, Vec::new());
        }

        println!("---- Threads Report");

        for en in &self.processed_entities {

            let index = en.thread_number;
            if let Some(vec) = display_entities.get_mut(&index) {
                vec.push(en);
            }

        }

        let mut dbg_string = String::new();

        for (index, entities) in display_entities.iter() {

            dbg_string.push_str(&format!("Thread number {}\n", index));
            dbg_string.push_str("Processed files: ");

            for en in entities {
                dbg_string.push_str(&format!("\"{}\"\n", en.name));
            }
            dbg_string.push_str("\n");

        }


        write!(f,
        "---- Directory Report\nTotal Files: {}\nTotal Size: {}bytes\n{}",
        self.total_files, self.total_size, dbg_string
        )
    }
}


fn process_entities(entries: Vec<DirEntry>, no_threads: i32) -> Result<Report, String> {

    if no_threads <= 0 {
        return Err("Number of threads not specified".to_string());
    }

    let mut threads_vec: Vec<JoinHandle<()>> = vec![];
    let entities: Arc<Mutex<Vec<Entity>>> = Arc::new(Mutex::new(vec![]));

    let arc_entries = Arc::new(Mutex::new(entries));
    let processed_entities = Arc::new(Mutex::new(0usize));

    let files_count = Arc::new(Mutex::new(0usize));
    let bytes_count = Arc::new(Mutex::new(0usize));

    for iterator in 0..no_threads {

        let processed_entities = Arc::clone(&processed_entities);
        let arc_entries_clone = Arc::clone(&arc_entries);
        let entities_clone = Arc::clone(&entities);
        let files_count = Arc::clone(&files_count);
        let bytes_count = Arc::clone(&bytes_count);

        let thread = std::thread::spawn(move || {

            loop {
                let mut guard = processed_entities.lock().unwrap();
                let index = *guard;
                let dir_entry = arc_entries_clone.lock().unwrap();

                if index >= dir_entry.len() {
                    break;
                }

                *guard += 1;
                drop(guard);
                let entity = Entity::from_dir_entry(&dir_entry[index], iterator as u32);

                drop(dir_entry);

                if let Some(e) = entity {
                    let mut entity_guard = entities_clone.lock().unwrap();
                    let mut file_guard = files_count.lock().unwrap();
                    let mut bytes_guard = bytes_count.lock().unwrap();
                    *file_guard += 1;
                    *bytes_guard += e.size as usize;
                    entity_guard.push(e);
                }

            }



        });


        threads_vec.push(thread);
    }


    for th in threads_vec {
        th.join().unwrap();
    }

    println!("Done!!");
    let total_size = *bytes_count.lock().unwrap();
    let total_files = *files_count.lock().unwrap();
    let final_entities = Arc::try_unwrap(entities).unwrap().into_inner().unwrap();
    Ok(
        Report::new(
            total_size as u64,
            total_files as u64,
            final_entities
        )
    )
}



fn main() {

    let args = Args::parse();

    let dir = std::fs::read_dir(&args.path);

    let mut entries: Vec<DirEntry> = vec![];

    match dir {
        Ok(entry) => {
            for e in entry {
                if e.is_ok() {
                    let obj = e.unwrap();
                    entries.push(obj);
                }

            }
        }
        Err(err) => {
            println!("Error happened: {:?}", err);
        }
    }

    let rsl = process_entities(entries, 4);

    match rsl {
        Ok(rsl) => {
            println!("{rsl}");
        }
        Err(e) => {
            println!("Error: {e}");
        }
    }

}
